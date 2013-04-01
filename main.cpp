/*
 * main.cpp
 *
 *  Created on: 2013-4-1
 *      Author: keym
 */

#include<iostream>
#include"database_pool.h"
#include<unistd.h>
#include<mysql/mysql.h>

void query(MYSQL* mysql)
{
	int qury = mysql_query(mysql,"select * from namepsw");
	if(qury)
	{
		std::cerr<<"cannot query"<<std::endl;
	}
	else
	{
		MYSQL_RES *res;
		MYSQL_ROW row;
		res = mysql_store_result(mysql);
		if(res)
		{
			int num = mysql_num_rows(res);
			std::cout<<"mun:"<<num<<std::endl;
			while((row = mysql_fetch_row(res)) != NULL)
			{
				unsigned i;
				for(i = 0;i < mysql_num_fields(res);++ i)
				{
					std::cout<<i<<":"<<row[i]<<std::endl;
				}
			}
		}
		else
		{
			std::cerr<<mysql_errno(mysql)<<std::endl;
		}
		mysql_free_result(res);
	}
}

int main()
{
	database_pool *dp = database_pool::getInstance();
	dp->database_init("localhost","root","123456","userinfo",4);
	pid_t pid;
	if((pid = fork()) > 0)
	{
		MYSQL *mysql1 = dp->database_get();
		query(mysql1);
		dp->database_ret(mysql1);
	}
	else if(pid == 0)
	{
		MYSQL *mysql1 = dp->database_get();
		query(mysql1);
		dp->database_ret(mysql1);
	}
	return 0;
}
