#include "http-server/http-server.hpp"
#include "http-server/log.hpp"
#include "http-server/db/sqlite.hpp"
#include "http-server/ulid.hpp"
#include "http-server/tcp-client.hpp"
#include "http-server/utils.hpp"

#include <any>
#include <variant>

struct App {
  http::db::Sqlite db{ { .db_name = "gallery.db" } };
} g_app;

//-----------------------------------------------------------------------

struct ExampleHandler : public http::HttpRequestHandler {
  static inline http::HttpMethod method = http::HttpMethod::GET;
  static inline const char*      path   = "/api/example";

  ~ExampleHandler() override {
    http::g_log->debug("~ExampleHandler");
  }

  HandleResult handle() override {
    return g_app.db
        .with_connection(
            [](sqlite::database& db) {
              int result;
              db << "select count(*) from tests where age > ? ;"
                 << 18 >>
                  [&](int count) {
                    result = count;
                  };
              return result;
            })
        .then([](int count) {
          http::HttpResponse response{};
          response.status(http::HttpStatusCode::OK)
              << "tests rows count: " << count << "\r\n"
              << "some ulid: " << http::UlidGenerator::main().generate().str() << "\r\n";
          return response;
        });
  }
};

//-----------------------------------------------------------------------

struct FillHandler : public http::HttpRequestHandler {
  static inline http::HttpMethod method = http::HttpMethod::GET;
  static inline const char*      path   = "/api/fill";

  ~FillHandler() override = default;

  HandleResult handle() override {
    return g_app.db
        .with_connection(
            [](sqlite::database& db) {
              db << "create table if not exists tests ("
                    "   _id integer primary key autoincrement not null,"
                    "   age int,"
                    "   name text,"
                    "   weight real"
                    ");";
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 20 << u"bob" << 83.25;
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 21 << u"alice" << 56.4;
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 22 << u"dungeon master" << 98.0;
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 24 << u"stefany" << 44.25;
              return nullptr;
            })
        .then([](nullptr_t) {
          http::HttpResponse response{};
          response.status(http::HttpStatusCode::Created).with_default_status_message();
          return response;
        });
  }
};

//-----------------------------------------------------------------------

struct TestPartsHandler : public http::HttpRequestHandler {
  static inline http::HttpMethod method = http::HttpMethod::POST;
  static inline const char*      path   = "/api/part/{int}/{string}";

  int         part{ 0 };
  std::string name{};
  std::string password{};
  std::string username{};

  ~TestPartsHandler() override = default;

  bool preprocess() override {
    if (!unwrap_url(part, name)) {
      return false;
    }
    if (!request.body) {
      return false;
    }
    if (request.body->type() != http::HttpBodyType::Json) {
      return false;
    }

    auto body = dynamic_cast<http::HttpBodyJson*>(request.body.get());
    password  = body->object["password"].get<std::string>();
    username  = body->object["username"].get<std::string>();

    return true;
  }

  HandleResult handle() override {
    return cti::async([this] {
      http::HttpResponse response{};
      response.status(http::HttpStatusCode::OK)
          << "hello from parts handler!\r\n"
          << "current part: " << part << ", name: " << name << "\r\n"
          << "password: '" << password << "'\r\n"
          << "username: '" << username << "'\r\n";
      return response;
    });
  }
};

//-----------------------------------------------------------------------


#define mIProtoKeyEnum(X)          \
  X(IProtoKey_RequestType, 0x00)   \
  X(IProtoKey_Sync, 0x01)          \
  X(IProtoKey_ReplicaId, 0x02)     \
  X(IProtoKey_Lsn, 0x03)           \
  X(IProtoKey_Timestamp, 0x04)     \
  X(IProtoKey_SchemaVersion, 0x05) \
  X(IProtoKey_ServerVersion, 0x06) \
  X(IProtoKey_GroupId, 0x07)       \
  X(IProtoKey_Tsn, 0x08)           \
  X(IProtoKey_Flags, 0x09)         \
  X(IProtoKey_StreamId, 0x0a)      \
  X(IProtoKey_SpaceId, 0x10)       \
  X(IProtoKey_IndexId, 0x11)       \
  X(IProtoKey_Limit, 0x12)         \
  X(IProtoKey_Offset, 0x13)        \
  X(IProtoKey_Iterator, 0x14)      \
  X(IProtoKey_IndexBase, 0x15)     \
  X(IProtoKey_Key, 0x20)           \
  X(IProtoKey_Tuple, 0x21)         \
  X(IProtoKey_FunctionName, 0x22)  \
  X(IProtoKey_UserName, 0x23)      \
  X(IProtoKey_InstanceUuid, 0x24)  \
  X(IProtoKey_ClusterUuid, 0x25)   \
  X(IProtoKey_Vclock, 0x26)        \
  X(IProtoKey_Expr, 0x27)          \
  X(IProtoKey_Ops, 0x28)           \
  X(IProtoKey_Ballot, 0x29)        \
  X(IProtoKey_TupleMeta, 0x2a)     \
  X(IProtoKey_Options, 0x2b)       \
  X(IProtoKey_Data, 0x30)          \
  X(IProtoKey_Error24, 0x31)       \
  X(IProtoKey_Metadata, 0x32)      \
  X(IProtoKey_BindMetadata, 0x33)  \
  X(IProtoKey_BindCount, 0x34)     \
  X(IProtoKey_SqlText, 0x40)       \
  X(IProtoKey_SqlBind, 0x41)       \
  X(IProtoKey_SqlInfo, 0x42)       \
  X(IProtoKey_StmtId, 0x43)        \
  X(IProtoKey_ReplicaAnon, 0x50)   \
  X(IProtoKey_IdFilter, 0x51)      \
  X(IProtoKey_Error, 0x52)         \
  X(IProtoKey_Term, 0x53)          \
  X(IProtoKey_Version, 0x54)       \
  X(IProtoKey_Features, 0x55)

mDeclareEnum(IProtoKey, mIProtoKeyEnum);

#define mIProtoTypeEnum(X)           \
  X(IProtoType_Ok, 0)                \
  X(IProtoType_Select, 1)            \
  X(IProtoType_Insert, 2)            \
  X(IProtoType_Replace, 3)           \
  X(IProtoType_Update, 4)            \
  X(IProtoType_Delete, 5)            \
  X(IProtoType_Call16, 6)            \
  X(IProtoType_Auth, 7)              \
  X(IProtoType_Eval, 8)              \
  X(IProtoType_Upsert, 9)            \
  X(IProtoType_Call, 10)             \
  X(IProtoType_Execute, 11)          \
  X(IProtoType_Nop, 12)              \
  X(IProtoType_Prepare, 13)          \
  X(IProtoType_Begin, 14)            \
  X(IProtoType_Commit, 15)           \
  X(IProtoType_Rollback, 16)         \
  X(IProtoType_TypeStatMax, 17)      \
  X(IProtoType_Raft, 30)             \
  X(IProtoType_RaftPromote, 31)      \
  X(IProtoType_RaftDemote, 32)       \
  X(IProtoType_RaftConfirm, 40)      \
  X(IProtoType_RaftRollback, 41)     \
  X(IProtoType_Ping, 64)             \
  X(IProtoType_Join, 65)             \
  X(IProtoType_Subscribe, 66)        \
  X(IProtoType_VoteDeprecated, 67)   \
  X(IProtoType_Vote, 68)             \
  X(IProtoType_FetchSnapshot, 69)    \
  X(IProtoType_Register, 70)         \
  X(IProtoType_JoinMeta, 71)         \
  X(IProtoType_JoinSnapshot, 72)     \
  X(IProtoType_Id, 73)               \
  X(IProtoType_VyIndexRunInfo, 100)  \
  X(IProtoType_VyIndexPageInfo, 101) \
  X(IProtoType_VyRunRowIndex, 102)   \
  X(IProtoType_Chunk, 128)           \
  X(IProtoType_TypeError, 1 << 15)

mDeclareEnum(IProtoType, mIProtoTypeEnum);

#define mIProtoErrorEnum(X)                    \
  X(IProtoError_Unknown)                       \
  X(IProtoError_IllegalParams)                 \
  X(IProtoError_MemoryIssue)                   \
  X(IProtoError_TupleFound)                    \
  X(IProtoError_TupleNotFound)                 \
  X(IProtoError_Unsupported)                   \
  X(IProtoError_Nonmaster)                     \
  X(IProtoError_Readonly)                      \
  X(IProtoError_Injection)                     \
  X(IProtoError_CreateSpace)                   \
  X(IProtoError_SpaceExists)                   \
  X(IProtoError_DropSpace)                     \
  X(IProtoError_AlterSpace)                    \
  X(IProtoError_IndexType)                     \
  X(IProtoError_ModifyIndex)                   \
  X(IProtoError_LastDrop)                      \
  X(IProtoError_TupleFormatLimit)              \
  X(IProtoError_DropPrimaryKey)                \
  X(IProtoError_KeyPartType)                   \
  X(IProtoError_ExactMatch)                    \
  X(IProtoError_InvalidMsgpack)                \
  X(IProtoError_ProcRet)                       \
  X(IProtoError_TupleNotArray)                 \
  X(IProtoError_FieldType)                     \
  X(IProtoError_IndexPartTypeMismatch)         \
  X(IProtoError_UpdateSplice)                  \
  X(IProtoError_UpdateArgType)                 \
  X(IProtoError_FormatMismatchIndexPart)       \
  X(IProtoError_UnknownUpdateOp)               \
  X(IProtoError_UpdateField)                   \
  X(IProtoError_FunctionTxActive)              \
  X(IProtoError_KeyPartCount)                  \
  X(IProtoError_ProcLua)                       \
  X(IProtoError_NoSuchProc)                    \
  X(IProtoError_NoSuchTrigger)                 \
  X(IProtoError_NoSuchIndexId)                 \
  X(IProtoError_NoSuchSpace)                   \
  X(IProtoError_NoSuchFieldNo)                 \
  X(IProtoError_ExactFieldCount)               \
  X(IProtoError_FieldMissing)                  \
  X(IProtoError_WalIo)                         \
  X(IProtoError_MoreThanOneTuple)              \
  X(IProtoError_AccessDenied)                  \
  X(IProtoError_CreateUser)                    \
  X(IProtoError_DropUser)                      \
  X(IProtoError_NoSuchUser)                    \
  X(IProtoError_UserExists)                    \
  X(IProtoError_PasswordMismatch)              \
  X(IProtoError_UnknownRequestType)            \
  X(IProtoError_UnknownSchemaObject)           \
  X(IProtoError_CreateFunction)                \
  X(IProtoError_NoSuchFunction)                \
  X(IProtoError_FunctionExists)                \
  X(IProtoError_BeforeReplaceRet)              \
  X(IProtoError_MultistatementTransaction)     \
  X(IProtoError_TriggerExists)                 \
  X(IProtoError_UserMax)                       \
  X(IProtoError_NoSuchEngine)                  \
  X(IProtoError_ReloadCfg)                     \
  X(IProtoError_Cfg)                           \
  X(IProtoError_SavepointEmptyTx)              \
  X(IProtoError_NoSuchSavepoint)               \
  X(IProtoError_UnknownReplica)                \
  X(IProtoError_ReplicasetUuidMismatch)        \
  X(IProtoError_InvalidUuid)                   \
  X(IProtoError_ReplicasetUuidIsRo)            \
  X(IProtoError_InstanceUuidMismatch)          \
  X(IProtoError_ReplicaIdIsReserved)           \
  X(IProtoError_InvalidOrder)                  \
  X(IProtoError_MissingRequestField)           \
  X(IProtoError_Identifier)                    \
  X(IProtoError_DropFunction)                  \
  X(IProtoError_IteratorType)                  \
  X(IProtoError_ReplicaMax)                    \
  X(IProtoError_InvalidXlog)                   \
  X(IProtoError_InvalidXlogName)               \
  X(IProtoError_InvalidXlogOrder)              \
  X(IProtoError_NoConnection)                  \
  X(IProtoError_Timeout)                       \
  X(IProtoError_ActiveTransaction)             \
  X(IProtoError_CursorNoTransaction)           \
  X(IProtoError_CrossEngineTransaction)        \
  X(IProtoError_NoSuchRole)                    \
  X(IProtoError_RoleExists)                    \
  X(IProtoError_CreateRole)                    \
  X(IProtoError_IndexExists)                   \
  X(IProtoError_SessionClosed)                 \
  X(IProtoError_RoleLoop)                      \
  X(IProtoError_Grant)                         \
  X(IProtoError_PrivGranted)                   \
  X(IProtoError_RoleGranted)                   \
  X(IProtoError_PrivNotGranted)                \
  X(IProtoError_RoleNotGranted)                \
  X(IProtoError_MissingSnapshot)               \
  X(IProtoError_CantUpdatePrimaryKey)          \
  X(IProtoError_UpdateIntegerOverflow)         \
  X(IProtoError_GuestUserPassword)             \
  X(IProtoError_TransactionConflict)           \
  X(IProtoError_UnsupportedPriv)               \
  X(IProtoError_LoadFunction)                  \
  X(IProtoError_FunctionLanguage)              \
  X(IProtoError_RtreeRect)                     \
  X(IProtoError_ProcC)                         \
  X(IProtoError_UnknownRtreeIndexDistanceType) \
  X(IProtoError_Protocol)                      \
  X(IProtoError_UpsertUniqueSecondaryKey)      \
  X(IProtoError_WrongIndexRecord)              \
  X(IProtoError_WrongIndexParts)               \
  X(IProtoError_WrongIndexOptions)             \
  X(IProtoError_WrongSchemaVersion)            \
  X(IProtoError_MemtxMaxTupleSize)             \
  X(IProtoError_WrongSpaceOptions)             \
  X(IProtoError_UnsupportedIndexFeature)       \
  X(IProtoError_ViewIsRo)                      \
  X(IProtoError_NoTransaction)                 \
  X(IProtoError_System)                        \
  X(IProtoError_Loading)                       \
  X(IProtoError_ConnectionToSelf)              \
  X(IProtoError_KeyPartIsTooLong)              \
  X(IProtoError_Compression)                   \
  X(IProtoError_CheckpointInProgress)          \
  X(IProtoError_SubStmtMax)                    \
  X(IProtoError_CommitInSubStmt)               \
  X(IProtoError_RollbackInSubStmt)             \
  X(IProtoError_Decompression)                 \
  X(IProtoError_InvalidXlogType)               \
  X(IProtoError_AlreadyRunning)                \
  X(IProtoError_IndexFieldCountLimit)          \
  X(IProtoError_LocalInstanceIdIsReadOnly)     \
  X(IProtoError_BackupInProgress)              \
  X(IProtoError_ReadViewAborted)               \
  X(IProtoError_InvalidIndexFile)              \
  X(IProtoError_InvalidRunFile)                \
  X(IProtoError_InvalidVylogFile)              \
  X(IProtoError_CascadeRollback)               \
  X(IProtoError_VyQuotaTimeout)                \
  X(IProtoError_PartialKey)                    \
  X(IProtoError_TruncateSystemSpace)           \
  X(IProtoError_LoadModule)                    \
  X(IProtoError_VinylMaxTupleSize)             \
  X(IProtoError_WrongDdVersion)                \
  X(IProtoError_WrongSpaceFormat)              \
  X(IProtoError_CreateSequence)                \
  X(IProtoError_AlterSequence)                 \
  X(IProtoError_DropSequence)                  \
  X(IProtoError_NoSuchSequence)                \
  X(IProtoError_SequenceExists)                \
  X(IProtoError_SequenceOverflow)              \
  X(IProtoError_NoSuchIndexName)               \
  X(IProtoError_SpaceFieldIsDuplicate)         \
  X(IProtoError_CantCreateCollation)           \
  X(IProtoError_WrongCollationOptions)         \
  X(IProtoError_NullablePrimary)               \
  X(IProtoError_NoSuchFieldNameInSpace)        \
  X(IProtoError_TransactionYield)              \
  X(IProtoError_NoSuchGroup)                   \
  X(IProtoError_SqlBindValue)                  \
  X(IProtoError_SqlBindType)                   \
  X(IProtoError_SqlBindParameterMax)           \
  X(IProtoError_SqlExecute)                    \
  X(IProtoError_UpdateDecimalOverflow)         \
  X(IProtoError_SqlBindNotFound)               \
  X(IProtoError_ActionMismatch)                \
  X(IProtoError_ViewMissingSql)                \
  X(IProtoError_ForeignKeyConstraint)          \
  X(IProtoError_NoSuchModule)                  \
  X(IProtoError_NoSuchCollation)               \
  X(IProtoError_CreateFkConstraint)            \
  X(IProtoError_DropFkConstraint)              \
  X(IProtoError_NoSuchConstraint)              \
  X(IProtoError_ConstraintExists)              \
  X(IProtoError_SqlTypeMismatch)               \
  X(IProtoError_RowidOverflow)                 \
  X(IProtoError_DropCollation)                 \
  X(IProtoError_IllegalCollationMix)           \
  X(IProtoError_SqlNoSuchPragma)               \
  X(IProtoError_SqlCantResolveField)           \
  X(IProtoError_IndexExistsInSpace)            \
  X(IProtoError_InconsistentTypes)             \
  X(IProtoError_SqlSyntaxWithPos)              \
  X(IProtoError_SqlStackOverflow)              \
  X(IProtoError_SqlSelectWildcard)             \
  X(IProtoError_SqlStatementEmpty)             \
  X(IProtoError_SqlKeywordIsReserved)          \
  X(IProtoError_SqlSyntaxNearToken)            \
  X(IProtoError_SqlUnknownToken)               \
  X(IProtoError_SqlParserGeneric)              \
  X(IProtoError_SqlAnalyzeArgument)            \
  X(IProtoError_SqlColumnCountMax)             \
  X(IProtoError_HexLiteralMax)                 \
  X(IProtoError_IntLiteralMax)                 \
  X(IProtoError_SqlParserLimit)                \
  X(IProtoError_IndexDefUnsupported)           \
  X(IProtoError_CkDefUnsupported)              \
  X(IProtoError_MultikeyIndexMismatch)         \
  X(IProtoError_CreateCkConstraint)            \
  X(IProtoError_CkConstraintFailed)            \
  X(IProtoError_SqlColumnCount)                \
  X(IProtoError_FuncIndexFunc)                 \
  X(IProtoError_FuncIndexFormat)               \
  X(IProtoError_FuncIndexParts)                \
  X(IProtoError_NoSuchFieldName)               \
  X(IProtoError_FuncWrongArgCount)             \
  X(IProtoError_BootstrapReadonly)             \
  X(IProtoError_SqlFuncWrongRetCount)          \
  X(IProtoError_FuncInvalidReturnType)         \
  X(IProtoError_SqlParserGenericWithPos)       \
  X(IProtoError_ReplicaNotAnon)                \
  X(IProtoError_CannotRegister)                \
  X(IProtoError_SessionSettingInvalidValue)    \
  X(IProtoError_SqlPrepare)                    \
  X(IProtoError_WrongQueryId)                  \
  X(IProtoError_SequenceNotStarted)            \
  X(IProtoError_NoSuchSessionSetting)          \
  X(IProtoError_UncommittedForeignSyncTxns)    \
  X(IProtoError_SyncMasterMismatch)            \
  X(IProtoError_SyncQuorumTimeout)             \
  X(IProtoError_SyncRollback)                  \
  X(IProtoError_TupleMetadataIsTooBig)         \
  X(IProtoError_XlogGap)                       \
  X(IProtoError_TooEarlySubscribe)             \
  X(IProtoError_SqlCantAddAutoinc)             \
  X(IProtoError_QuorumWait)                    \
  X(IProtoError_InterferingPromote)            \
  X(IProtoError_ElectionDisabled)              \
  X(IProtoError_TxnRollback)                   \
  X(IProtoError_NotLeader)                     \
  X(IProtoError_SyncQueueUnclaimed)            \
  X(IProtoError_SyncQueueForeign)              \
  X(IProtoError_UnableToProcessInStream)       \
  X(IProtoError_UnableToProcessOutOfStream)

mDeclareSimpleEnum(IProtoError, mIProtoErrorEnum);

struct TarantoolError {
  std::string  type;
  std::string  file;
  unsigned int line;
  std::string  message;
  unsigned int err_no;
  unsigned int err_code; // IProtoError

  void unpack(const msgpack::object& object) {
    if (object.type != msgpack::type::MAP) {
      throw msgpack::type_error();
    }
    for (size_t i = 0; i < object.via.map.size; i++) {
      auto& kv = object.via.map.ptr[i];
      switch (kv.key.as<unsigned int>()) {
        case 0: type = kv.val.as<decltype(type)>(); break;
        case 1: file = kv.val.as<decltype(file)>(); break;
        case 2: line = kv.val.as<decltype(line)>(); break;
        case 3: message = kv.val.as<decltype(message)>(); break;
        case 4: err_no = kv.val.as<decltype(err_no)>(); break;
        case 5: err_code = kv.val.as<decltype(err_code)>(); break;
      }
    }
  }

  std::string to_string() {
    std::stringstream ss;
    ss << "  type: " << type << "\n";
    ss << "  file: " << file << "\n";
    ss << "  line: " << line << "\n";
    ss << "  message: " << message << "\n";
    ss << "  err_no: " << err_no << "\n";
    ss << "  err_code: " << err_code << "\n";
    return ss.str();
  }
};

struct ExampleTcpClientUser : public http::ITcpClientUser {
  enum class State {
    PreHello,
    Waiting,
    Ready,
  } state = State::PreHello;

#pragma pack(push, 1)
  struct HelloPacket {
    char greeting_line_1[64];
    char greeting_line_2[64];

    std::string_view get_greeting_line_1() {
      auto line = std::string_view{ greeting_line_1, sizeof(greeting_line_1) };
      return line.substr(0, line.find('\n'));
    }

    std::string_view get_greeting_line_2() {
      auto line = std::string_view{ greeting_line_2, sizeof(greeting_line_2) };
      return line.substr(0, line.find('\n'));
    }
  };
#pragma pack(pop)

  ~ExampleTcpClientUser() override {
    http::g_log->info("~ExampleTcpClientUser");
  }

  void on_data(char* data, size_t size) override {
    std::string_view incoming{ data, size };

    switch (state) {
      case State::PreHello: {
        http::g_log->debug("incoming.size(): {}, sizeof(HelloPacket): {}", incoming.size(), sizeof(HelloPacket));
        assert(incoming.size() == sizeof(HelloPacket));
        auto hello = reinterpret_cast<HelloPacket*>(data);
        http::g_log->debug("recieved hello from tarantool instance");
        http::g_log->debug("  .greeting_line_1: {}", hello->get_greeting_line_1());
        http::g_log->debug("  .greeting_line_2: {}", hello->get_greeting_line_2());
        state = State::Ready;
        [[fallthrough]];
      }

      case State::Ready: {
        write_some_data();
        state = State::Waiting;
        break;
      }

      case State::Waiting: {
        http::g_log->info("read_callback on waiting! {}", incoming);

        msgpack::unpacker pac;
        pac.reserve_buffer(incoming.size());
        memcpy(pac.buffer(), incoming.data(), incoming.size());
        pac.buffer_consumed(incoming.size());

        msgpack::object_handle oh;

        assert(pac.next(oh));
        {
          std::stringstream ss;
          ss << oh.get();
          http::g_log->info(" <size>: {}", ss.str());
        }

        assert(pac.next(oh));
        {
          msgpack::type::assoc_vector<unsigned int, unsigned int> dst;
          oh.get().convert(dst);

          for (auto [key, value] : dst) {
            if (key == IProtoKey_RequestType) {
              if ((value & IProtoType_TypeError) != 0) {
                auto err = (IProtoError) (value ^ IProtoType_TypeError); // remove error bit
                http::g_log->info(" <header> {}: (error) {}", to_string((IProtoKey) key), to_string(err));
              } else {
                http::g_log->info(" <header> {}: {}", to_string((IProtoKey) key), to_string((IProtoType) value));
              }
            } else {
              http::g_log->info(" <header> {}: {}", to_string((IProtoKey) key), value);
            }
          }
        }

        assert(pac.next(oh));
        {
          msgpack::type::assoc_vector<unsigned int, msgpack::object> dst;
          oh.get().convert(dst);

          bool is_error = false;
          for (const auto& [key, value] : dst) {
            if (key == IProtoKey_Error) {
              auto errors = value.as<msgpack::type::assoc_vector<unsigned int,
                                                                 std::vector<msgpack::object>>>();
              for (auto& [key, value] : errors) {
                if (key == 0) { // stack error
                  for (auto& error : value) {
                    TarantoolError err_typed;
                    err_typed.unpack(error);
                    http::g_log->info(" <body>: (error) {}", err_typed.to_string());
                  }
                }
              }

              is_error = true;

            } else if (key == IProtoKey_Error24) {
              auto message = value.as<std::string>();
              http::g_log->info(" <body>: (error) {}", message);
              is_error = true;
            }
          }

          if (!is_error) {
            std::stringstream ss;
            ss << oh.get();
            http::g_log->info(" <body>: (unknown) {}", ss.str());
          }
        }

        assert(!pac.next(oh));
        http::g_log->info("deserialize done");

        state = State::Ready;
        break;
      }

      default: break;
    }
  }

  void write_some_data() {
    const auto func_name = std::string_view{ "example_get" };

    auto buffer = msgpack::sbuffer{};
    auto packer = msgpack::packer{ buffer };

    // header
    {
      packer.pack_map(2);

      packer.pack_unsigned_int(IProtoKey_RequestType);
      packer.pack_unsigned_int(IProtoType_Call);

      packer.pack_unsigned_int(IProtoKey_Sync);
      packer.pack_unsigned_int(0);
    }

    // body
    {
      packer.pack_map(2);

      packer.pack_unsigned_int(IProtoKey_FunctionName);
      packer.pack_str(func_name.size());
      packer.pack_str_body(func_name.data(), func_name.size());

      packer.pack_unsigned_int(IProtoKey_Tuple);
      packer.pack_array(1);
      packer.pack_unsigned_int(127);
    }

    auto size_buffer = msgpack::sbuffer{};
    auto size_packer = msgpack::packer{ size_buffer };
    size_packer.pack_unsigned_int(buffer.size());

    client_->write(const_cast<char*>(size_buffer.data()), size_buffer.size());
    client_->write(const_cast<char*>(buffer.data()), buffer.size());
  }

  void on_hello() override {
    // const char* msg = "hello!\r\n";
    // client_->write(const_cast<char*>(msg), strlen(msg));
  }
};

int main(int, char**) {
  http::TcpClient client{ std::make_unique<ExampleTcpClientUser>() };
  client.connect("127.0.0.1", 3301u);

  // http::HttpServer server;
  // server.add_handler<ExampleHandler>();
  // server.add_handler<TestPartsHandler>();
  // server.add_handler<FillHandler>();
  // server.listen("127.0.0.1", 5000);
  return http::run_main_loop();
}
