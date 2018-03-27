#pragma region Includes

	#include <iostream>
	#include <limits>
	#include <direct.h>
	#include <map>

	#include <string>
	#include <filesystem>

	#include "redbase.h"
	#include "rm.h"
	#include "sm.h"
	#include "ql.h"

#pragma endregion

using namespace std;
namespace fs = experimental::filesystem;

std::map<int, std::string> databases_options;

void displayOptions() {
	cout << "Please make your selection -->\n";
	cout << "    1 - Create database\n";
	cout << "    2 - Open database\n";
	cout << "    3 - Quit\n";
	cout << "Option: ";
}

int getInput() {
	int choice;
	cin >> choice;

	if (!cin.good()) {
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		choice = -100;
	}

	return choice;
}

int initDBMS() {

	char command[MAXSTRINGLEN] = "mkdir ";

	strcat_s(command, "databases");
	if (system(command) != 0) {
		//cerr << "Cannot create database directory: databases.\n";
		//exit(1);
	}

	return 0;
}

int listAllDatabases() {
	int countdb = 0;
	string path = ".\\databases\\";
	for (auto & subfolder : fs::directory_iterator(path)) {
		countdb++;
		databases_options.insert(std::make_pair(countdb, subfolder.path().string().replace(0, 12, "")));
		cout << "    " << countdb << " - " << subfolder.path().string().replace(0, 12, "") << "\n";
	}

	return countdb;
}

void openDatabase(char *dbname) {
	PF_Manager pfm;
	RM_Manager rmm(pfm);
	IX_Manager ixm(pfm);
	SM_Manager smm(ixm, rmm);
	QL_Manager qlm(smm, ixm, rmm);

	char path[255] = ".\\databases\\";
	strcat_s(path, dbname);
	if (smm.OpenDb(path) != OK_RC) {
		cout << "Unable to open database\n";
	}

	RBparse(pfm, smm, qlm);

	// close the database
	if (smm.CloseDb() != OK_RC) {
		cout << "Unable to close database\n";
	}
	cout << "Finished...\n";
}

int createDatabase(char *dbname) {
	char command[255] = "mkdir .\\databases\\";
	char path[255] = ".\\databases\\";

	strcat_s(command, dbname);

	if (system(command) != 0) {
		cerr << " cannot create directory: " << dbname << "\n";
		exit(1);
	}	

	strcat_s(path, dbname);

	if (_chdir(path) < 0) {
		cerr << " chdir error to " << dbname << "\n";
		exit(1);
	}

	PF_Manager pfm;
	RM_Manager rmm(pfm);

	char relcat[] = "relcat";
	// create the file for storing relation catalogs
	if (rmm.CreateFile(relcat, sizeof(RelationInfo)) != 0) {
		cerr << "Error while creating relcat\n";
	}

	char attrcat[] = "attrcat";
	// create the file for storing attributes 
	if (rmm.CreateFile(attrcat, sizeof(DataAttrInfo)) != 0) {
		cerr << "Error while creating attrcat\n";
	}

	// open the files 
	RM_FileHandle relation_file, attribute_file;
	if ((rmm.OpenFile(relcat, relation_file)) ||
		(rmm.OpenFile(attrcat, attribute_file))) {
		cout << "Error while creating database\n";
		return 1;
	}

	RelationInfo relations_TableStruct, attributes_TableStruct;
	RID temp_rid;

	strncpy_s(relations_TableStruct.rel_name, relcat, MAXNAME + 1);
	relations_TableStruct.tuple_size = sizeof(RelationInfo);
	relations_TableStruct.num_attr = 4;
	relations_TableStruct.index_num = -1; //means that has no index

	strncpy_s(attributes_TableStruct.rel_name, attrcat, MAXNAME + 1);
	attributes_TableStruct.tuple_size = sizeof(DataAttrInfo);
	attributes_TableStruct.num_attr = 6;
	attributes_TableStruct.index_num = -1;

	if (relation_file.InsertRec((char*)&relations_TableStruct, temp_rid) ||
		relation_file.InsertRec((char*)&attributes_TableStruct, temp_rid)) {
		cout << "Error while creating database\n";
		return 1;
	}

	// Update Attrcat
	DataAttrInfo attr[10];

	const char *relation_attr_names[] = { "relName", "tupleLength", "attrCount", "indexNo" };
	const char *attribute_attr_names[] = { "relName", "attrName", "offset", "attrType", "attrLength", "indexNo" };

	int tuple_size = (char*)&relations_TableStruct.tuple_size - (char*)&relations_TableStruct;
	int attribute_lenght = sizeof(AttrType);

	int relation_offsets[] = { 0, tuple_size, tuple_size + 4, tuple_size + 8 };

	int attributes_offsets[] = { 0, tuple_size, 2 * tuple_size,2 * tuple_size + 4, 2 * tuple_size + 4 + attribute_lenght, 2 * tuple_size + 8 + attribute_lenght };

	int relation_lengths[] = { tuple_size, 4, 4, 4 };
	int attribute_lengths[] = { tuple_size, tuple_size, 4, attribute_lenght, 4, 4 };

	AttrType relation_attr_types[] = { STRING, INT, INT, INT };
	AttrType attribute_attr_types[] = { STRING, STRING, INT, INT, INT, INT };

	// Write to the file his structure
	for (int i = 0; i < 4; i++) {
		strncpy_s(attr[i].relName, relcat, MAXNAME + 1);
		strncpy_s(attr[i].attrName, (char*)relation_attr_names[i], MAXNAME + 1);
		attr[i].offset = relation_offsets[i];
		attr[i].attrType = relation_attr_types[i];
		attr[i].attrLength = relation_lengths[i];
		attr[i].indexNo = -1;
		if (attribute_file.InsertRec((char*)&attr[i], temp_rid)) {
			cout << "Error while creating database\n";
		}
	}

	for (int i = 0; i < 6; i++) {
		strncpy_s(attr[i].relName, attrcat, MAXNAME + 1);
		strncpy_s(attr[i].attrName, (char*)attribute_attr_names[i], MAXNAME + 1);

		attr[i].offset = attributes_offsets[i];
		attr[i].attrType = attribute_attr_types[i];
		attr[i].attrLength = attribute_lengths[i];
		attr[i].indexNo = -1;
		if (attribute_file.InsertRec((char*)&attr[i], temp_rid)) {
			cout << "Error while creating database\n";
		}
	}

	if (relation_file.ForcePages(ALL_PAGES) || attribute_file.ForcePages(ALL_PAGES)) {
		cout << "Error while creating database\n";
		return 1;
	}
	if (rmm.CloseFile(relation_file) || rmm.CloseFile(attribute_file)) {
		cout << "Error while creating database\n";
		return 1;
	}

	cout << "The database has been created successfully!\n\n";

	return -1;
}

int main(int argc, char *argv[]) {
	char dbname[MAXNAME + 1] = "";
	int choice = 0;

	initDBMS();

	do {
		displayOptions();
		choice = getInput();
		switch (choice) {
		case 1:
			cout << "Please type the name of the database: ";
			cin >> dbname;
			createDatabase(dbname);
			system("PAUSE");
			system("cls");
			break;
		case 2:
			cout << "To open the database, please select one of the listed bellow:\n";
			if (listAllDatabases() == 0) {
				cout << "Sorry no databases found!\n";
				system("PAUSE");
				system("cls");
				break;
			}
			cout << "Open database id: ";
			cin >> choice;
			cout << "\n\nOpening database #: " << choice << "\n";
			system("cls");
			char dbSelected[MAXNAME + 1];
			strcpy_s(dbSelected, MAXNAME+1, databases_options.at(choice).c_str());
			openDatabase(dbSelected);
			break;
		case 3:
			cout << "Bye!\n\n";
			break;
		case (-100):
			cout << "Invalid option. Please select one of the options presented!\nHit any key\n";
			system("PAUSE");
			system("cls");
		default:
			system("cls");
			displayOptions();
			choice = getInput();
		}
	} while (choice != 3);

	system("PAUSE");
	return EXIT_SUCCESS;
}