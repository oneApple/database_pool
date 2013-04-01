/*
 * database_pool.cpp
 *
 *  Created on: 2013-4-1
 *      Author: keym
 */

#include"database_pool.h"
#include"guard.h"
#include<stdlib.h>

database_pool *database_pool::instance = NULL;
database_pool::release database_pool::re;
//静态成员变量必须在实现文件中定义
database_pool *database_pool::getInstance()//获取实例句柄
{
	if(instance == NULL)
	{
		instance = new database_pool;
	}
	return instance;
}

void *database_pool::create_con()//创建一个句柄
{
	MYSQL * mysql;
	if((mysql = mysql_init(NULL)) != NULL)
	{
		if(mysql_real_connect(mysql,this->host.c_str(),this->name.c_str(),this->psw.c_str(),this->database.c_str(),MYSQL_PORT,NULL,0) == NULL)
		{
			return NULL;
		}
	}
	return mysql;
}

int database_pool::database_init(std::string host,std::string name,std::string psw,std::string database,int max)
{
	this->host = host;
	this->name = name;
	this->psw = psw;
	this->database = database;
	this->maxsize = max;
	this->cursize = max / 4;
	int i;
	if(pthread_mutex_init(&this->mutex,NULL) != 0)
	{
		return 0;
	}
	guard g(&this->mutex);
	if(! g.getstate())
	{
		return 0;
	}
	for(i = 0;i < this->cursize;++ i)//创建最大句柄数四分之一的句柄
	{
		MYSQL *mysql;
		if((mysql = (MYSQL*)this->create_con()) == NULL)
		{
			return 0;
		}
		sqldata *data = new sqldata;
		data->s = IDLE;
		data->sql = mysql;
		this->vdata.push_back(data);
		this->mdata[data->sql] = i;
	}
	return 1;
}

MYSQL *database_pool::database_get()//获取句柄，如果当前使用的句柄数超过当前的数目，则将当前数目增加为2倍
{
	guard g(&this->mutex);
	if(! g.getstate())
	{
		return 0;
	}
	if(this->curnum == this->cursize)
	{
		if(this->cursize == this->maxsize)
		{
			return NULL;
		}
		int i;
		this->cursize *= 2;
		for(i = this->cursize / 2;i < this->cursize;++ i)
		{
			MYSQL *mysql;
			if((mysql = (MYSQL*)this->create_con()) == NULL)
			{
				return NULL;
			}
			sqldata *data = new sqldata;
			data->s = IDLE;
			data->sql = mysql;
			this->vdata.push_back(data);
			this->mdata[data->sql] = i;
		}
	}
	int i;
	for(i = 0;i < this->cursize;++ i)
	{
		sqldata* data = this->vdata[i];
		if(data->s == IDLE)
		{
			++ this->curnum;
			data->s = BUSY;
			return data->sql;
		}
	}
	return NULL;
}

void database_pool::database_ret(MYSQL *sql)//回收句柄，如果正在使用的句柄数减少的当前数目的四分之一，则将数目//将少到原来的一半
{
	guard g(&this->mutex);
	if(! g.getstate())
	{
		return;
	}
	std::map<MYSQL *,int>::iterator it;
	it = this->mdata.find(sql);
	if(it == this->mdata.end())
	{
		return;
	}
	int index = it->second;
	sqldata *data = this->vdata[index];
	if(data->s == IDLE)
	{
		return;
	}
	data->s = IDLE;
	--this->curnum;

	if(this->curnum == (this->cursize / 4))
	{
		this->mdata.clear();
		std::vector<sqldata *>::iterator iter = this->vdata.begin() + this->cursize / 2;
		std::vector<sqldata *> vtemp(this->vdata.begin(),iter);
		int i;
		iter = vtemp.begin();
		for(i = this->cursize / 2;i < this->cursize;++ i)
		{
			sqldata* data = this->vdata[i];
			if(data->s == BUSY)
			{
				while((*iter)->s != IDLE)
				{
					++ iter;
				}
				mysql_close((*iter)->sql);
				(*iter)->sql = data->sql;
			}
			delete this->vdata[i];
		}
		this->vdata.clear();
		this->vdata.assign(vtemp.begin(),vtemp.end());
		this->cursize = this->cursize / 2;
		for(i = 0;i < this->cursize;++ i)
		{
			this->mdata[this->vdata[i]->sql] = i;
		}
	}
}

void database_pool::destory()
{
	if(database_pool::instance != NULL)
	{
		std::vector<sqldata *>::iterator iter = this->vdata.begin();
		while(iter != this->vdata.end())
		{
			if((*iter)->s == BUSY)
			{
				mysql_close((*iter)->sql);
				(*iter)->s = IDLE;
			}
			delete *iter;
			++ iter;
		}
		pthread_mutex_destroy(&this->mutex);
		database_pool::instance = NULL;
	}
}
