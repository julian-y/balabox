#include <stdio.h>
#include "mysql_helper.hpp"

#include <my_global.h>
#include <mysql.h>

int main(void)
{
    MySQLHelper h;
    if (h.connect() == 0) {
        printf("Successfully connected!\n");
    }

    if(h.close() == 0) {
        printf("Successfully closed!\n");
    }
}