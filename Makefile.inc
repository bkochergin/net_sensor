DB_INCLUDES!=find /usr/local/include -type d -name "db[0-9]" -or -name "db[0-9][0-9]" | sort | tail -n 1
DB_LIBS!=find /usr/local/lib -type d -name "db[0-9]" -or -name "db[0-9][0-9]" | sort | tail -n 1
