#include <iostream>
#include <string>
#include <sstream>
#include <conio.h>
#include <list>


#include "mysql\mysql.h"
#pragma comment (lib,"libmysql.lib")

//#include <my_global.h>
//#include <mysql.h>

using namespace std;

#define MAX_PARAM 10

struct Recipes {
	string ingredient;
	int quantity;
};


int main ()
{

	// define mysql connection variables
	MYSQL *conn;	
	MYSQL_RES *res;
	MYSQL_ROW row;

	string param;

	cout << "start program" << endl;
	
	// read mysql connection	
	cout << "Please input mysql information hostname, port, username, password, database: " << endl;

	string hostname = "localhost";
	int port = 3306;
	string username = "root";
	string password = "";
	string dbname = "store";
	
	conn = mysql_init(NULL);

	while(true)
	{		
		// read command line
		//getline(cin, param); 
		//if( param.compare("q") == 0 )
		//	break;

		// split command string
		//string arr[MAX_PARAM];
		//int param_count = 0;
		//stringstream ssin(param);
		//while (ssin.good() && param_count < MAX_PARAM){
		//	ssin >> arr[param_count];
		//	++param_count;
		//}

		// check param count
		//if( param_count < 4 )	// if not provide sufficient information about connection
		//{
		//	cout << "Please input correct value" << endl;
		//	continue;
		//}

		//hostname = arr[0];
		//port = stoi(arr[1]);
		//username = arr[2];
		//password = "";
		//dbname = "";
		//if( param_count < 5 ) // empty password
		//	dbname = arr[3];
		//else // non empty password
		//{
		//	password = arr[3];
		//	dbname = arr[4];
		//}

		// connect mysql
		if (!mysql_real_connect(conn, hostname.c_str(), username.c_str(), password.c_str(), "", port, NULL, 0))
		{
			cout << "Can not connect db!!!, Please input again:" << endl;
			continue;
		}

		cout << "db is connected successfully" << endl;
		break;
	}
	
	char query[1000];

	// create database
	sprintf(query, "CREATE DATABASE IF NOT EXISTS %s", dbname.c_str()); 				
	mysql_query(conn, query);

	sprintf(query, "USE %s", dbname.c_str()); 				
	mysql_query(conn, query);

	cout << "db is created successfully" << endl;

	// create tables
	sprintf(query, "CREATE TABLE IF NOT EXISTS `recipes` ( "
		"`id` int(11) unsigned NOT NULL AUTO_INCREMENT, "
		"`recipe_name` varchar(50) DEFAULT NULL, "
		"`ingredient` varchar(50) DEFAULT NULL, "
		"`quantity` int(10) DEFAULT NULL, "
		"PRIMARY KEY (`id`) "
		") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
	mysql_query(conn, query);
	cout << "`recipes` table is created successfully" << endl;

	sprintf(query, "CREATE TABLE IF NOT EXISTS `inventory` ( "
		"`id` int(11) unsigned NOT NULL AUTO_INCREMENT, "
		"`ingredient` varchar(50) DEFAULT NULL, "
		"`quantity` int(10) DEFAULT '0', "
		"PRIMARY KEY (`id`) "
		") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
	mysql_query(conn, query);
	cout << "`grade` table is created successfully" << endl;

	cout << "1. Create a recipe or add ingredient to a recipe" << endl;
	cout << "2. List a recipe’s ingredients" << endl;
	cout << "3. Buy all recipe ingredients from the store" << endl;
	cout << "4. Add ingredients to the store" << endl;
	cout << "5. Quit" << endl;

	int choice1 = 0;
	
	while(true)
	{
		cin >> choice1;

		string recipe_name = "";
		string ingredient = "";
		int quantity = 0;

		list<Recipes> list1 = list<Recipes>();

		if( choice1 == 1 )	// Add receipe data
		{
			cout << "Recipe Name: ";
			cin >> recipe_name;
			cout << "Ingredient: ";
			cin >> ingredient;
			cout << "Quantity: ";
			cin >> quantity;

			sprintf(query, "INSERT INTO recipes (recipe_name, ingredient, quantity) VALUES ('%s', '%s', '%d')", recipe_name.c_str(), ingredient.c_str(), quantity); 				
			mysql_query(conn, query);
		}

		if( choice1 == 2 ) // List a recips data
		{
			cout << "Recipe Name: ";
			cin >> recipe_name;

			sprintf(query, "SELECT * FROM recipes WHERE recipe_name = '%s'", recipe_name.c_str());
			mysql_query(conn, query);
			res = mysql_use_result(conn);

			while((row=mysql_fetch_row(res))!=NULL)
				cout << row[1] << " " << row[2] << endl;				

			mysql_free_result(res);

		}
		
		if( choice1 == 3 ) // Buy recipes
		{		
			cout << "Recipe Name: ";
			cin >> recipe_name;

			sprintf(query, "SELECT * FROM recipes WHERE recipe_name = '%s'", recipe_name.c_str());
			mysql_query(conn, query);
			res = mysql_use_result(conn);

			Recipes data;
			while((row=mysql_fetch_row(res))!=NULL)
			{
				data.ingredient = row[2];
				data.quantity = atoi(row[3]);
				list1.push_back(data);
			}
				
			mysql_free_result(res);

			sprintf(query, "LOCK TABLE inventory WRITE;");
			mysql_query(conn, query);

			bool inventory_valid = true;
			std::list<Recipes>::iterator it;
			for (it = list1.begin(); it != list1.end(); ++it)
			{
				sprintf(query, "SELECT * FROM inventory WHERE ingredient = '%s'", it->ingredient.c_str());
				mysql_query(conn, query);
				res = mysql_use_result(conn);

				int inventory_quantity = 0;
				if((row=mysql_fetch_row(res))!=NULL)
				{
					inventory_quantity = atoi(row[2]);				
				}

				if( inventory_quantity < it->quantity )
				{					
					inventory_valid = false;
					mysql_free_result(res);
					break;
				}
				mysql_free_result(res);
			}

			if( inventory_valid == false )
			{
				cout << "Failure of purchase" << endl;
				continue;
			}

			for (it = list1.begin(); it != list1.end(); ++it)
			{
				sprintf(query, "UPDATE inventory SET quantity = quantity - %d WHERE ingredient = '%s'", it->quantity, it->ingredient.c_str()); 				
				mysql_query(conn, query);
			}

			sprintf(query, "UNLOCK TABLES;");
			mysql_query(conn, query);
		}

		if( choice1 == 4 )
		{			
			cout << "Ingredient: ";
			cin >> ingredient;
			cout << "Quantity: ";
			cin >> quantity;

			sprintf(query, "SELECT * FROM inventory WHERE ingredient = '%s'", ingredient.c_str());
			mysql_query(conn, query);
			res = mysql_use_result(conn);

			bool exist = false;			
			if((row=mysql_fetch_row(res))!=NULL)
				exist = true;				
			
			mysql_free_result(res);

			if( exist == false )
				sprintf(query, "INSERT INTO inventory (ingredient, quantity) VALUES ('%s', '%d')", ingredient.c_str(), quantity); 				
			else
				sprintf(query, "UPDATE inventory SET quantity = quantity + %d WHERE ingredient = '%s'", quantity, ingredient.c_str()); 				
			
			mysql_query(conn, query);			
		}

		if( choice1 == 5 )
			break;
	}

	mysql_close(conn);

	cout << "end program";
}

