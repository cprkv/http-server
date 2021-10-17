#pragma once
#include "http-server/pch.hpp"
#include "http-server/utils.hpp"

namespace http::db::tarantool {

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

} // namespace http