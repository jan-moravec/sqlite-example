#include <iostream>
#include <sqlite3.h> 
#include <iomanip>
#include <vector>

static sqlite3 *db_object;
static const char *db_name = "database.db"; 

int callback(void *data, int argc, char **argv, char **azColName);
int execute_sql(const char *sql_statement);

int create_table_simple();
int populate_table_simple();
int select_simple();
int create_table_advanced();
int populate_table_advanced();
int select_advanced();

int main(int argc, char* argv[]) 
{
	int ret;

	// Open or create database db_name, get database connection object db_object
	ret = sqlite3_open_v2(db_name, &db_object, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

	if (ret != SQLITE_OK) {
		std::cerr << __PRETTY_FUNCTION__ << ": Error opening database " <<  db_name << ":" << std::endl;
		std::cerr << "\t- " << sqlite3_errmsg(db_object) << std::endl; 
		return -1;
	}

	std::cout << __PRETTY_FUNCTION__ << ": Database " << db_name << " opened successfully." << std::endl;

	// Some simple routine
	create_table_simple();
	populate_table_simple();
	select_simple();

	// More advanced routine
	create_table_advanced();
	populate_table_advanced();
	select_advanced();

	// Close the database
	ret = sqlite3_close(db_object);
	if (ret != SQLITE_OK) {
		std::cerr << __PRETTY_FUNCTION__ << ": Error closing database " <<  db_name << ":" << std::endl;
		std::cerr << "\t- " << sqlite3_errmsg(db_object) << std::endl;
		return -1;
	}

	return 0;
}

int callback_print(void *user_data, int columns_count, char **row_values, char **column_names)
{
	unsigned &counter = *( static_cast<unsigned *>(user_data) );

	if (counter == 0) {
		for (int i = 0; i < columns_count; ++i) {
			std::cout << std::setw(15) << column_names[i];
		}
		std::cout << std::endl;
	}

	for (int i = 0; i < columns_count; ++i) {
		std::cout << std::setw(15) << row_values[i];
	}
	std::cout << std::endl;

	counter++;
	return 0;
}

int execute_sql(const char *sql_statement)
{
	int ret;
	char *error_message;
	unsigned counter = 0;

	// Execute SQL statement
	ret = sqlite3_exec(db_object, sql_statement, callback_print, &counter, &error_message);

	if (ret != SQLITE_OK) {
		std::cerr << __PRETTY_FUNCTION__ << ": SQL statement error:" << std::endl;
		//std::cerr << sql_statement << std::endl;
		std::cerr << "\t- " << error_message << std::endl;;
		sqlite3_free(error_message);
		return -1;
	}
	
	return 0;
}

int create_table_simple()
{
	std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
	const char *sql_statement = \
	"CREATE TABLE company ("  \
	"id 			INTEGER PRIMARY KEY     AUTOINCREMENT," \
	"name           TEXT    				NOT NULL," \
	"age            INTEGER     			NOT NULL," \
	"address        CHAR(50)," \
	"salary         REAL );";

	return execute_sql(sql_statement);
}

int populate_table_simple()
{
	std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
	const char *sql_statement = \
	"INSERT INTO company (name, age, address, salary) "  \
	"VALUES ('Paul', 32, 'California', 20000.00 ); " \
	"INSERT INTO company (name, age, address, salary) "  \
	"VALUES ('Allen', 25, 'Texas', 15000.00 ); "     \
	"INSERT INTO company (name, age, address, salary)" \
	"VALUES ('Teddy', 23, 'Norway', 25000.00 );" \
	"INSERT INTO company (name, age, address, salary)" \
	"VALUES ('Mark', 25, 'Rich-Mond ', 65000.00 );";

	return execute_sql(sql_statement);
}

int select_simple()
{
	std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
	const char *sql_statement = "SELECT * FROM company";

	return execute_sql(sql_statement);
}

int create_table_advanced()
{
	std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
	const char *sql_statement = \
	"CREATE TABLE generated_data ("  \
	"id 			INTEGER PRIMARY KEY     AUTOINCREMENT," \
	"name           TEXT    				NOT NULL," \
	"number         REAL );";

	return execute_sql(sql_statement);
}

void generate_random_text(char *s, const int len) {
    static const char alphanum[] = "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len-1; ++i) {
        s[i] = alphanum[std::rand() % (sizeof(alphanum) - 1)];
    }

    s[len-1] = 0;
}

int populate_table_advanced()
{
	std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
	int ret;
	sqlite3_stmt *sql_statement_object = nullptr;
	const char *sql_statement = "INSERT INTO generated_data (name, number) VALUES (?1, ?2);";

	ret = sqlite3_prepare_v2(db_object, sql_statement, -1, &sql_statement_object, nullptr);
	if (ret != SQLITE_OK) {
		std::cerr << __PRETTY_FUNCTION__ << ": SQL sqlite3_prepare_v2 error: " << ret << std::endl;
		return -1;
	}

	std::cout << __PRETTY_FUNCTION__ << ": The statement \"" << sql_statement << "\" has " << sqlite3_bind_parameter_count(sql_statement_object) << " parameters(s)." << std::endl;

	std::srand(std::time(0));
	for (int i = 0; i < 1000; ++i) {
		char text[5];
		generate_random_text(text, 5);

		ret = sqlite3_bind_text(sql_statement_object, 1, text, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			std::cerr << __PRETTY_FUNCTION__ << ": SQL sqlite3_bind_text error: " << ret << std::endl;
			return -1;
		}
		ret = sqlite3_bind_double(sql_statement_object, 2, (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX)) * 100000.0);
		if (ret != SQLITE_OK) {
			std::cerr << __PRETTY_FUNCTION__ << ": SQL sqlite3_bind_int error: " << ret << std::endl;
			return -1;
		}

		ret = sqlite3_step(sql_statement_object);
		if (ret != SQLITE_DONE) {
			std::cerr << __PRETTY_FUNCTION__ << ": SQL statement finished with error: " << ret << std::endl;
			return -1;
		}

		ret = sqlite3_reset(sql_statement_object);
		if (ret != SQLITE_OK) {
			std::cerr << __PRETTY_FUNCTION__ << ": SQL sqlite3_reset error: " << ret << std::endl;
			return -1;
		}
		//sqlite3_clear_bindings(sql_statement_object);
	}

	sqlite3_finalize(sql_statement_object);
	return 0;
}

int select_advanced()
{
	std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
	int ret;
	sqlite3_stmt *sql_statement_object = nullptr;
	const char *sql_statement = "SELECT id, name, number FROM generated_data WHERE number > ?1 ORDER BY number DESC LIMIT 10";

	ret = sqlite3_prepare_v2(db_object, sql_statement, -1, &sql_statement_object, nullptr);
	if (ret != SQLITE_OK) {
		std::cerr << __PRETTY_FUNCTION__ << ": SQL sqlite3_prepare_v2 error: " << ret << std::endl;
		return -1;
	}

	std::cout << __PRETTY_FUNCTION__ << ": The statement \"" << sql_statement << "\" has " << sqlite3_bind_parameter_count(sql_statement_object) << " parameters(s)." << std::endl;

	ret = sqlite3_bind_int(sql_statement_object, 1, 90000);
	if (ret != SQLITE_OK) {
		std::cerr << __PRETTY_FUNCTION__ << ": SQL sqlite3_bind_int error: " << ret << std::endl;
		return -1;
	}

	for (int i = 0; i < sqlite3_column_count(sql_statement_object); ++i) {
		std::cout << std::setw(15) << sqlite3_column_name(sql_statement_object, i);
	}
	std::cout << std::endl;

	ret = sqlite3_step(sql_statement_object);
	while (ret == SQLITE_ROW) {
		for (int i = 0; i < sqlite3_column_count(sql_statement_object); ++i) {
			std::cout << std::setw(15);
			switch (sqlite3_column_type(sql_statement_object, i)) {
			case SQLITE_INTEGER:
				std::cout << sqlite3_column_int(sql_statement_object, i);
				break;

			case SQLITE_FLOAT:
				std::cout << sqlite3_column_double(sql_statement_object, i);
				break;

  			case SQLITE_TEXT:
				std::cout << sqlite3_column_text(sql_statement_object, i);
				break;

			case SQLITE_BLOB:
				std::cout << "SQLITE_BLOB";
				break;

			case SQLITE_NULL:
				std::cout << "SQLITE_NULL";
				break;
			}
		}

		std::cout << std::endl;
		ret = sqlite3_step(sql_statement_object);
	}

	if (ret != SQLITE_DONE) {
		std::cerr << __PRETTY_FUNCTION__ << ": SQL statement finished with error: " << ret << std::endl;
		return -1;
	}

	sqlite3_finalize(sql_statement_object);
	return 0;
}