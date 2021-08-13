//////////////////////////////////////////////////////////////////////
// duckdbAPI.h
//////////////////////////////////////////////////////////////////////

#if !defined(__DUCKDB_API_H__)
#define __DUCKDB_API_H__

#include <SQLAPI.h>
#include <samisc.h>

#include <duckdb.h>

typedef duckdb_state (*duckdb_open_t)(const char* path, duckdb_database* out_database);
typedef void (*duckdb_close_t)(duckdb_database* database);
typedef duckdb_state (*duckdb_connect_t)(duckdb_database database, duckdb_connection* out_connection);
typedef void (*duckdb_disconnect_t)(duckdb_connection* connection);
typedef duckdb_state (*duckdb_query_t)(duckdb_connection connection, const char* query, duckdb_result* out_result);
typedef void (*duckdb_destroy_result_t)(duckdb_result* result);
typedef const char* (*duckdb_column_name_t)(duckdb_result* result, idx_t col);
typedef bool (*duckdb_value_boolean_t)(duckdb_result* result, idx_t col, idx_t row);
typedef int8_t (*duckdb_value_int8_t)(duckdb_result* result, idx_t col, idx_t row);
typedef int16_t (*duckdb_value_int16_t)(duckdb_result* result, idx_t col, idx_t row);
typedef int32_t (*duckdb_value_int32_t)(duckdb_result* result, idx_t col, idx_t row);
typedef int64_t (*duckdb_value_int64_t)(duckdb_result* result, idx_t col, idx_t row);
typedef uint8_t(*duckdb_value_uint8_t)(duckdb_result* result, idx_t col, idx_t row);
typedef uint16_t(*duckdb_value_uint16_t)(duckdb_result* result, idx_t col, idx_t row);
typedef uint32_t(*duckdb_value_uint32_t)(duckdb_result* result, idx_t col, idx_t row);
typedef uint64_t(*duckdb_value_uint64_t)(duckdb_result* result, idx_t col, idx_t row);
typedef float (*duckdb_value_float_t)(duckdb_result* result, idx_t col, idx_t row);
typedef double (*duckdb_value_double_t)(duckdb_result* result, idx_t col, idx_t row);
typedef char* (*duckdb_value_varchar_t)(duckdb_result* result, idx_t col, idx_t row);
typedef duckdb_blob (*duckdb_value_blob_t)(duckdb_result* result, idx_t col, idx_t row);
typedef duckdb_state (*duckdb_prepare_t)(duckdb_connection connection, const char* query,
	duckdb_prepared_statement* out_prepared_statement);
typedef duckdb_state (*duckdb_nparams_t)(duckdb_prepared_statement prepared_statement, idx_t* nparams_out);
typedef duckdb_state (*duckdb_bind_boolean_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx, bool val);
typedef duckdb_state (*duckdb_bind_int8_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int8_t val);
typedef duckdb_state (*duckdb_bind_int16_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int16_t val);
typedef duckdb_state (*duckdb_bind_int32_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int32_t val);
typedef duckdb_state (*duckdb_bind_int64_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int64_t val);
typedef duckdb_state (*duckdb_bind_float_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx, float val);
typedef duckdb_state (*duckdb_bind_double_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx, double val);
typedef duckdb_state (*duckdb_bind_varchar_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	const char* val);
typedef duckdb_state (*duckdb_bind_null_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx);
typedef duckdb_state (*duckdb_bind_blob_t)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	const void* data, idx_t length);
typedef duckdb_state (*duckdb_execute_prepared_t)(duckdb_prepared_statement prepared_statement,
	duckdb_result* out_result);
typedef void (*duckdb_destroy_prepare_t)(duckdb_prepared_statement* prepared_statement);

// API declarations
class SQLAPI_API duckdbAPI : public IsaAPI
{
public:
	duckdbAPI();

public:
	duckdb_open_t duckdb_open;
	duckdb_close_t duckdb_close;
	duckdb_connect_t duckdb_connect;
	duckdb_disconnect_t duckdb_disconnect;
	duckdb_query_t duckdb_query;
	duckdb_destroy_result_t duckdb_destroy_result;
	duckdb_column_name_t duckdb_column_name;
	duckdb_value_boolean_t duckdb_value_boolean;
	duckdb_value_int8_t duckdb_value_int8;
	duckdb_value_int16_t duckdb_value_int16;
	duckdb_value_int32_t duckdb_value_int32;
	duckdb_value_int64_t duckdb_value_int64;
	duckdb_value_uint8_t duckdb_value_uint8;
	duckdb_value_uint16_t duckdb_value_uint16;
	duckdb_value_uint32_t duckdb_value_uint32;
	duckdb_value_uint64_t duckdb_value_uint64;
	duckdb_value_float_t duckdb_value_float;
	duckdb_value_double_t duckdb_value_double;
	duckdb_value_varchar_t duckdb_value_varchar;
	duckdb_value_blob_t duckdb_value_blob;
	duckdb_prepare_t duckdb_prepare;
	duckdb_nparams_t duckdb_nparams;
	duckdb_bind_boolean_t duckdb_bind_boolean;
	duckdb_bind_int8_t duckdb_bind_int8;
	duckdb_bind_int16_t duckdb_bind_int16;
	duckdb_bind_int32_t duckdb_bind_int32;
	duckdb_bind_int64_t duckdb_bind_int64;
	duckdb_bind_float_t duckdb_bind_float;
	duckdb_bind_double_t duckdb_bind_double;
	duckdb_bind_varchar_t duckdb_bind_varchar;
	duckdb_bind_null_t duckdb_bind_null;
	duckdb_bind_blob_t duckdb_bind_blob;
	duckdb_execute_prepared_t duckdb_execute_prepared;
	duckdb_destroy_prepare_t duckdb_destroy_prepare;

public:
	virtual void InitializeClient(const SAOptions *pOptions);
	virtual void UnInitializeClient(const SAPI *pSAPI, const SAOptions* pOptions);

	virtual long GetClientVersion() const;

	virtual ISAConnection *NewConnection(SAConnection *pConnection);

protected:
	void  *m_hLibrary;
	SAMutex m_loaderMutex;

	void ResetAPI();
	void LoadAPI();
};

class SQLAPI_API duckdbConnectionHandles : public saConnectionHandles
{
public:
	duckdbConnectionHandles();
	duckdb_database db;
};

class SQLAPI_API duckdbCommandHandles : public saCommandHandles
{
public:
	duckdbCommandHandles();
	
	duckdb_connection conn;
	duckdb_result result;
	duckdb_prepared_statement stmt;
};

#endif //__DUCKDB_API_H__
