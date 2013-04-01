/*
 * database_pool.h
 *
 *  Created on: 2013-4-1
 *      Author: keym
 */

#ifndef DATABASE_POOL_H_
#define DATABASE_POOL_H_

//数据库连接池
#include<mysql/mysql.h>
#include"unCopy.h"
#include<string>
#include<vector>
#include<map>
#include<pthread.h>

enum STATE{IDLE,BUSY};
typedef enum STATE STATE;

struct sqldata{
	MYSQL* sql;
	STATE s;
};
typedef struct sqldata sqldata;

class database_pool : private uncopy{
private:
	std::string name;//用户名
	std::string psw;//用户密码
	std::string database;//数据库名
	std::string host;//主机名
	std::vector<sqldata *> vdata;
	std::map<MYSQL *,int> mdata;
	pthread_mutex_t mutex;
	int curnum;
	int maxsize;
	int cursize;
	static database_pool *instance;
private:
	explicit database_pool():curnum(0){}
	void destory();
	void* create_con();
private:
	class release{
	public:
		~release()
		{
			if(database_pool::instance != NULL)
			{
				delete database_pool::instance;
			}
		}
	};
	static release re;//释放静态句柄
public:
	static database_pool* getInstance();//获取唯一实例
	int database_init(std::string host,std::string name,std::string psw,std::string database,int max);//初始化
	MYSQL *database_get();//获取句柄
	void database_ret(MYSQL *sql);//回收句柄
	~database_pool()//析构
	{
		this->destory();
	}
};

#endif /* DATABASE_POOL_H_ */
