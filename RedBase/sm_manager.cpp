#pragma region Includes
	#include <io.h>
	#include <iostream>
	#include <fstream>
	#include <sstream>
	#include <algorithm>
	#include <direct.h>

	#include "sm.h"
	#include "rm.h"
#pragma endregion

using namespace std;

SM_Manager::SM_Manager(IX_Manager &ixm, RM_Manager &rmm) {
	ixman = &ixm;
	rmman = &rmm;
	isOpen = false;
	SHOW_ALL_PLANS = false;
	SORT_RES = 0;
}

SM_Manager::~SM_Manager() {
	if (isOpen) {
		rmman->CloseFile(relcat);
		rmman->CloseFile(attrcat);
	}
}

RC SM_Manager::Set(const char *paramName, const char *value) {
	if (strcmp(paramName, "allplans") == 0 && strcmp(value, "1") == 0) {
		SHOW_ALL_PLANS = true;
		cout << "Will show all intermediate plans" << endl;
		return OK_RC;
	}
	else if (strcmp(paramName, "allplans") == 0 && strcmp(value, "0") == 0) {
		SHOW_ALL_PLANS = false;
		cout << "Will not show all plans" << endl;
		return OK_RC;
	}
	else if (strcmp(paramName, "sort") == 0 && strcmp(value, "2") == 0) {
		SORT_RES = 2;
		cout << "Will sort all resuls descending" << endl;
		return OK_RC;
	}
	else if (strcmp(paramName, "sort") == 0 && strcmp(value, "1") == 0) {
		SORT_RES = 1;
		cout << "Will sort all resuls ascending" << endl;
		return OK_RC;
	}
	else if (strcmp(paramName, "sort") == 0 && strcmp(value, "0") == 0) {
		SORT_RES = 0;
		cout << "Will not sort all results" << endl;
		return OK_RC;
	}
	return SM_NOT_IMPLEMENTED;
}

RC SM_Manager::OpenDb(const char *dbName) {
	RC WARN = SM_OPEN_WARN, ERR = SM_OPEN_ERR;
	if (isOpen) return WARN;
	SM_ErrorForward(_chdir(dbName));
	char relcatfile[] = "relcat";
	char attrcatfile[] = "attrcat";
	SM_ErrorForward(rmman->OpenFile(relcatfile, relcat));
	SM_ErrorForward(rmman->OpenFile(attrcatfile, attrcat));
	isOpen = true;
	return OK_RC;
}

RC SM_Manager::CloseDb() {
	RC WARN = SM_CLOSE_WARN, ERR = SM_CLOSE_ERR;
	if (!isOpen) return WARN;
	SM_ErrorForward(rmman->CloseFile(relcat));
	SM_ErrorForward(rmman->CloseFile(attrcat));
	SM_ErrorForward(_chdir(".."));
	isOpen = false;
	return OK_RC;
}

RC SM_Manager::CreateTable(const char *relName,
	int        attrCount,
	AttrInfo   *attributes) {
	RC WARN = SM_CREATE_WARN, ERR = SM_CREATE_ERR;

	if (attrCount < 1) return SM_BAD_INPUT;
	for (int i = 0; i < attrCount; i++) {
		for (int j = i + 1; j < attrCount; j++) {
			if (strcmp(attributes[i].attrName, attributes[j].attrName) == 0)
				return SM_BAD_INPUT;
		}
	}

	if (!isOpen) return SM_DB_CLOSED;
	if ((strcmp(relName, "relcat") == 0)
		|| (strcmp(relName, "attrcat") == 0)) {
		return SM_BAD_INPUT;
	}
	if (_access(relName, 0) == 0) return SM_DUPLICATE_RELATION;
	if (strlen(relName) > MAXNAME) return SM_BAD_INPUT;
	int index = strlen(relName) - 1;
	while (index >= 0 && isdigit((unsigned char)relName[index])) index--;
	if (relName[index] == '.' && index > 0) return SM_BAD_INPUT;
	int recSize = 0;
	for (int i = 0; i < attrCount; i++) {
		if (strlen(attributes[i].attrName) > MAXNAME) return SM_BAD_INPUT;
		if ((attributes[i].attrType < INT)
			|| (attributes[i].attrType > STRING)) return SM_BAD_INPUT;
		if ((attributes[i].attrType != STRING)
			&& (attributes[i].attrLength != 4)) return SM_BAD_INPUT;
		if ((attributes[i].attrType == STRING)
			&& ((attributes[i].attrLength < 1) ||
			(attributes[i].attrLength > MAXSTRINGLEN))) return SM_BAD_INPUT;
		recSize += attributes[i].attrLength;
	}
	SM_ErrorForward(rmman->CreateFile(relName, recSize));
	DataAttrInfo attr_desc;
	RID temp_rid;
	for (int i = 0, offset = 0; i < attrCount; i++) {
		strncpy_s(attr_desc.relName, relName, MAXNAME + 1);
		strncpy_s(attr_desc.attrName, attributes[i].attrName, MAXNAME + 1);
		attr_desc.offset = offset;
		offset += attributes[i].attrLength;
		attr_desc.attrType = attributes[i].attrType;
		attr_desc.attrLength = attributes[i].attrLength;
		attr_desc.indexNo = -1;
		if (attrcat.InsertRec((char*)&attr_desc, temp_rid)) {
			return SM_CREATE_ERR;
		}
	}
	RelationInfo relinfo;
	strncpy_s(relinfo.rel_name, relName, MAXNAME + 1);
	relinfo.tuple_size = recSize;
	relinfo.num_attr = attrCount;
	relinfo.index_num = -1;
	if (relcat.InsertRec((char*)&relinfo, temp_rid)) {
		return SM_CREATE_ERR;
	}
	int pnum;
	SM_ErrorForward(temp_rid.GetPageNum(pnum));
	SM_ErrorForward(relcat.ForcePages(ALL_PAGES));
	SM_ErrorForward(attrcat.ForcePages(ALL_PAGES));
	return OK_RC;
}

RC SM_Manager::DropTable(const char *relName) {
	RC WARN = SM_DROP_WARN, ERR = SM_DROP_ERR;
	if (!isOpen) return SM_DB_CLOSED;
	if ((strcmp(relName, "relcat") == 0)
		|| (strcmp(relName, "attrcat") == 0)) {
		return SM_BAD_INPUT;
	}
	if (_unlink(relName) < 0) return WARN;
	RM_Record rec;
	RID rid;
	SM_ErrorForward(getRelInfo(relName, rec));
	SM_ErrorForward(rec.GetRid(rid));
	SM_ErrorForward(relcat.DeleteRec(rid));
	RM_FileScan attrscan;
	SM_ErrorForward(attrscan.OpenScan(attrcat, STRING, MAXNAME + 1, 0, EQ_OP, (void*)relName, NO_HINT));
	DataAttrInfo* dinfo;
	char* dinfodata;
	while (attrscan.GetNextRec(rec) == OK_RC) {
		SM_ErrorForward(rec.GetData(dinfodata));
		dinfo = (DataAttrInfo*)dinfodata;
		if (dinfo->indexNo >= 0) {
			SM_ErrorForward(ixman->DestroyIndex(relName, dinfo->indexNo));
		}
		SM_ErrorForward(rec.GetRid(rid));
		SM_ErrorForward(attrcat.DeleteRec(rid));
	}
	SM_ErrorForward(attrscan.CloseScan());
	SM_ErrorForward(relcat.ForcePages(ALL_PAGES));
	SM_ErrorForward(attrcat.ForcePages(ALL_PAGES));
	return OK_RC;
}

RC SM_Manager::CreateIndex(const char *relName,
	const char *attrName) {
	RC WARN = SM_IXCREATE_WARN, ERR = SM_IXCREATE_ERR;
	if (!isOpen) return SM_DB_CLOSED;
	if (_access(relName, 0) != 0) return WARN;
	RelationInfo relinfo;
	DataAttrInfo dinfo;
	RM_Record relrec, attrec, datarec;
	SM_ErrorForward(getAttrInfo(relName, attrName, dinfo, attrec));
	if (dinfo.indexNo >= 0) return WARN;
	SM_ErrorForward(getRelInfo(relName, relrec));
	char *relinfodata;
	SM_ErrorForward(relrec.GetData(relinfodata));
	memcpy(&relinfo, relinfodata, sizeof(RelationInfo));
	relinfo.index_num++;
	memcpy(relinfodata, &relinfo, sizeof(RelationInfo));
	SM_ErrorForward(ixman->CreateIndex(relName, relinfo.index_num,
		dinfo.attrType, dinfo.attrLength));
	dinfo.indexNo = relinfo.index_num;
	char *dinfodata;
	SM_ErrorForward(attrec.GetData(dinfodata));
	memcpy(dinfodata, &dinfo, sizeof(DataAttrInfo));
	IX_IndexHandle ihandle;
	RM_FileHandle relation;
	RM_FileScan fscan;
	SM_ErrorForward(ixman->OpenIndex(relName, dinfo.indexNo, ihandle));
	SM_ErrorForward(rmman->OpenFile(relName, relation));
	SM_ErrorForward(fscan.OpenScan(relation, dinfo.attrType,
		dinfo.attrLength, dinfo.offset, NO_OP, 0, NO_HINT));
	char *data;
	RID rid;
	while (fscan.GetNextRec(datarec) == OK_RC) {
		SM_ErrorForward(datarec.GetData(data));
		SM_ErrorForward(datarec.GetRid(rid));
		SM_ErrorForward(ihandle.InsertEntry((void*)(data + dinfo.offset), rid));
	}
	SM_ErrorForward(fscan.CloseScan());
	SM_ErrorForward(rmman->CloseFile(relation));
	SM_ErrorForward(ihandle.ForcePages());
	SM_ErrorForward(ixman->CloseIndex(ihandle));
	SM_ErrorForward(attrcat.UpdateRec(attrec));
	SM_ErrorForward(relcat.UpdateRec(relrec));
	SM_ErrorForward(relcat.ForcePages(ALL_PAGES));
	SM_ErrorForward(attrcat.ForcePages(ALL_PAGES));
	return OK_RC;
}

RC SM_Manager::DropIndex(const char *relName,
	const char *attrName) {
	RC WARN = SM_IXDROP_WARN, ERR = SM_IXDROP_ERR;
	if (!isOpen) return SM_DB_CLOSED;
	if (_access(relName, 0) != 0) return WARN;
	DataAttrInfo dinfo;
	RM_Record rec;
	SM_ErrorForward(getAttrInfo(relName, attrName, dinfo, rec));
	if (dinfo.indexNo < 0) return WARN;
	SM_ErrorForward(ixman->DestroyIndex(relName, dinfo.indexNo));
	dinfo.indexNo = -1;
	char *dinfodata;
	SM_ErrorForward(rec.GetData(dinfodata));
	memcpy(dinfodata, &dinfo, sizeof(DataAttrInfo));
	SM_ErrorForward(attrcat.UpdateRec(rec));
	SM_ErrorForward(relcat.ForcePages(ALL_PAGES));
	SM_ErrorForward(attrcat.ForcePages(ALL_PAGES));
	return OK_RC;
}

RC SM_Manager::Load(const char *relName,
	const char *fileName) {
	RC WARN = SM_LOAD_WARN, ERR = SM_LOAD_ERR;
	if (!isOpen) return SM_DB_CLOSED;
	if (_access(relName, 0) != 0) return WARN;
	if (_access(fileName, 0) != 0) return WARN;
	RM_Record relrec, rec;
	RelationInfo* relinfo;
	char* relinfodata;
	SM_ErrorForward(getRelInfo(relName, relrec));
	SM_ErrorForward(relrec.GetData(relinfodata));
	relinfo = (RelationInfo*)relinfodata;
	vector<DataAttrInfo> attributes;
	SM_ErrorForward(getAttributes(relName, attributes));

	vector<int> ind;
	vector<IX_IndexHandle> ihandles(attributes.size());
	for (size_t i = 0; i < attributes.size(); i++) {
		if (attributes[i].indexNo >= 0) {
			ind.push_back(i);
			SM_ErrorForward(ixman->OpenIndex(relName,
				attributes[i].indexNo, ihandles[i]));
		}
	}
	RM_FileHandle relation;
	SM_ErrorForward(rmman->OpenFile(relName, relation));
	string line, word;
	char* buffer = new char[relinfo->tuple_size];
	RID record_rid;
	ifstream file(fileName);
	if (!file.is_open()) return WARN;
	stringstream ss;
	while (getline(file, line)) {
		ss << line;
		for (int i = 0; i < relinfo->num_attr; i++) {
			getline(ss, word, ',');
			if (attributes[i].attrType == INT) {
				int intatt = atoi(word.c_str());
				memcpy(buffer + attributes[i].offset,
					&intatt, attributes[i].attrLength);
			}
			else if (attributes[i].attrType == FLOAT) {
				double flatt = atof(word.c_str());
				memcpy(buffer + attributes[i].offset,
					&flatt, attributes[i].attrLength);
			}
			else if (attributes[i].attrType == STRING) {
				strncpy_s(buffer + attributes[i].offset, strlen(word.c_str()), word.c_str(), attributes[i].attrLength);
			}
			else {
				return ERR;
			}
		}
		ss.clear();
		SM_ErrorForward(relation.InsertRec(buffer, record_rid));
		for (size_t i = 0; i < ind.size(); i++) {
			SM_ErrorForward(ihandles[ind[i]].InsertEntry((void*)
				(buffer + attributes[ind[i]].offset), record_rid));
		}
	}
	file.close();
	delete[] buffer;
	SM_ErrorForward(rmman->CloseFile(relation));
	for (size_t i = 0; i < ind.size(); i++) {
		SM_ErrorForward(ixman->CloseIndex(ihandles[ind[i]]));
	}
	return OK_RC;
}

RC SM_Manager::Print(const char *relName) {
	RC WARN = SM_PRINT_WARN, ERR = SM_PRINT_ERR;
	if (!isOpen) return SM_DB_CLOSED;
	if (_access(relName, 0) != 0) return WARN;
	RM_Record rec;
	RelationInfo relinfo;
	char* relinfodata;
	SM_ErrorForward(getRelInfo(relName, rec));
	SM_ErrorForward(rec.GetData(relinfodata));
	memcpy(&relinfo, relinfodata, sizeof(RelationInfo));
	DataAttrInfo* attributes = new DataAttrInfo[relinfo.num_attr];
	RM_FileScan attrscan;
	SM_ErrorForward(attrscan.OpenScan(attrcat, STRING,
		MAXNAME + 1, 0, EQ_OP, (void*)relName, NO_HINT));
	char *dinfodata;
	for (int i = 0; i < relinfo.num_attr; i++) {
		SM_ErrorForward(attrscan.GetNextRec(rec));
		SM_ErrorForward(rec.GetData(dinfodata));
		memcpy(&attributes[i], dinfodata, sizeof(DataAttrInfo));
	}
	SM_ErrorForward(attrscan.CloseScan());

	Printer p(attributes, relinfo.num_attr);
	p.PrintHeader(cout);

	RM_FileHandle rfh;
	SM_ErrorForward(rmman->OpenFile(relName, rfh));

	RM_FileScan rfs;
	char *data;
	RC rc = OK_RC;

	SM_ErrorForward(rfs.OpenScan(rfh, INT, sizeof(int), 0, NO_OP, NULL));

	while (rc != RM_EOF) {
		rc = rfs.GetNextRec(rec);

		if (rc != 0 && rc != RM_EOF)
			return (rc);

		if (rc != RM_EOF) {
			SM_ErrorForward(rec.GetData(data));
			p.Print(cout, data);
		}
	}
	p.PrintFooter(cout);
	SM_ErrorForward(rfs.CloseScan());
	SM_ErrorForward(rmman->CloseFile(rfh));
	delete[] attributes;
	return OK_RC;
}

RC SM_Manager::Help() {
	RC WARN = SM_PRINT_WARN, ERR = SM_PRINT_ERR;
	if (!isOpen) return SM_DB_CLOSED;
	RM_Record rec;
	DataAttrInfo* attributes = new DataAttrInfo[6];
	strcpy_s(attributes[0].relName, "attrcat");
	strcpy_s(attributes[0].attrName, "relName");
	attributes[0].offset = 0;
	attributes[0].attrType = STRING;
	attributes[0].attrLength = MAXNAME + 1;
	attributes[0].indexNo = -1;
	strcpy_s(attributes[1].relName, "attrcat");
	strcpy_s(attributes[1].attrName, "attrName");
	attributes[1].offset = MAXNAME + 1;
	attributes[1].attrType = STRING;
	attributes[1].attrLength = MAXNAME + 1;
	attributes[1].indexNo = -1;
	strcpy_s(attributes[2].relName, "attrcat");
	strcpy_s(attributes[2].attrName, "offset");
	attributes[2].offset = 2 * MAXNAME + 2;
	attributes[2].attrType = INT;
	attributes[2].attrLength = 4;
	attributes[2].indexNo = -1;
	strcpy_s(attributes[3].relName, "attrcat");
	strcpy_s(attributes[3].attrName, "attrType");
	attributes[3].offset = 2 * MAXNAME + 6;
	attributes[3].attrType = INT;
	attributes[3].attrLength = 4;
	attributes[3].indexNo = -1;
	strcpy_s(attributes[4].relName, "attrcat");
	strcpy_s(attributes[4].attrName, "attrLength");
	attributes[4].offset = 2 * MAXNAME + 10;
	attributes[4].attrType = INT;
	attributes[4].attrLength = 4;
	attributes[4].indexNo = -1;
	strcpy_s(attributes[5].relName, "attrcat");
	strcpy_s(attributes[5].attrName, "indexNo");
	attributes[5].offset = 2 * MAXNAME + 14;
	attributes[5].attrType = INT;
	attributes[5].attrLength = 4;
	attributes[5].indexNo = -1;

	Printer p(attributes, 6);
	p.PrintHeader(cout);

	RM_FileScan relscan;
	SM_ErrorForward(relscan.OpenScan(relcat, STRING,
		MAXNAME + 1, 0, NO_OP, 0, NO_HINT));
	char *data;
	char *buffer = new char[MAXNAME + 9];
	RelationInfo relinfo;
	RC rc = OK_RC;

	while (rc != RM_EOF) {
		rc = relscan.GetNextRec(rec);

		if (rc != 0 && rc != RM_EOF)
			return (rc);

		if (rc != RM_EOF) {
			SM_ErrorForward(rec.GetData(data));
			memcpy(&relinfo, data, sizeof(RelationInfo));
			strncpy_s(buffer, MAXNAME + 1, relinfo.rel_name, MAXNAME + 1);
			memcpy(buffer + MAXNAME + 1, (void*)&relinfo.tuple_size, 4);
			memcpy(buffer + MAXNAME + 5, (void*)&relinfo.num_attr, 4);
			p.Print(cout, buffer);
		}
	}
	p.PrintFooter(cout);
	SM_ErrorForward(relscan.CloseScan());
	delete[] attributes;
	delete[] buffer;
	return OK_RC;
}

RC SM_Manager::Help(const char *relName) {
	RC WARN = SM_PRINT_WARN, ERR = SM_PRINT_ERR;
	if (!isOpen) return SM_DB_CLOSED;
	if (_access(relName, 0) != 0) return SM_RELATION_NOT_FOUND;
	RM_Record rec;

	DataAttrInfo* attributes = new DataAttrInfo[6];
	strcpy_s(attributes[0].relName, "attrcat");
	strcpy_s(attributes[0].attrName, "relName");
	attributes[0].offset = 0;
	attributes[0].attrType = STRING;
	attributes[0].attrLength = MAXNAME + 1;
	attributes[0].indexNo = -1;
	strcpy_s(attributes[1].relName, "attrcat");
	strcpy_s(attributes[1].attrName, "attrName");
	attributes[1].offset = MAXNAME + 1;
	attributes[1].attrType = STRING;
	attributes[1].attrLength = MAXNAME + 1;
	attributes[1].indexNo = -1;
	strcpy_s(attributes[2].relName, "attrcat");
	strcpy_s(attributes[2].attrName, "offset");
	attributes[2].offset = 2 * MAXNAME + 2;
	attributes[2].attrType = INT;
	attributes[2].attrLength = 4;
	attributes[2].indexNo = -1;
	strcpy_s(attributes[3].relName, "attrcat");
	strcpy_s(attributes[3].attrName, "attrType");
	attributes[3].offset = 2 * MAXNAME + 6;
	attributes[3].attrType = INT;
	attributes[3].attrLength = 4;
	attributes[3].indexNo = -1;
	strcpy_s(attributes[4].relName, "attrcat");
	strcpy_s(attributes[4].attrName, "attrLength");
	attributes[4].offset = 2 * MAXNAME + 10;
	attributes[4].attrType = INT;
	attributes[4].attrLength = 4;
	attributes[4].indexNo = -1;
	strcpy_s(attributes[5].relName, "attrcat");
	strcpy_s(attributes[5].attrName, "indexNo");
	attributes[5].offset = 2 * MAXNAME + 14;
	attributes[5].attrType = INT;
	attributes[5].attrLength = 4;
	attributes[5].indexNo = -1;

	Printer p(attributes, 6);
	p.PrintHeader(cout);

	RM_FileScan attrscan;
	SM_ErrorForward(attrscan.OpenScan(attrcat, STRING,
		MAXNAME + 1, 0, EQ_OP, (void*)relName, NO_HINT));
	char *data;
	char *buffer = new char[2 * MAXNAME + 18];
	DataAttrInfo dinfo;
	RC rc = OK_RC;

	while (rc != RM_EOF) {
		rc = attrscan.GetNextRec(rec);

		if (rc != 0 && rc != RM_EOF)
			return (rc);

		if (rc != RM_EOF) {
			SM_ErrorForward(rec.GetData(data));
			memcpy(&dinfo, data, sizeof(DataAttrInfo));
			strncpy_s(buffer, MAXNAME + 1, dinfo.relName, MAXNAME + 1);
			
			strncpy_s(buffer + MAXNAME + 1, MAXNAME + 1, dinfo.attrName, MAXNAME + 1);

			memcpy(buffer + 2 * MAXNAME + 2, (void*)&dinfo.offset, 4);
			memcpy(buffer + 2 * MAXNAME + 6, (void*)&dinfo.attrType, 4);
			memcpy(buffer + 2 * MAXNAME + 10, (void*)&dinfo.attrLength, 4);
			memcpy(buffer + 2 * MAXNAME + 14, (void*)&dinfo.indexNo, 4);
			p.Print(cout, buffer);
		}
	}
	p.PrintFooter(cout);
	SM_ErrorForward(attrscan.CloseScan());
	delete[] attributes;
	delete[] buffer;
	return OK_RC;
}

RC SM_Manager::getRelInfo(const char* relName, RM_Record &rec) {
	RC WARN = SM_RELATION_NOT_FOUND, ERR = SM_RELATION_NOT_FOUND;
	RM_FileScan relscan;
	SM_ErrorForward(relscan.OpenScan(relcat, STRING, MAXNAME + 1, 0, EQ_OP, (void*)relName, NO_HINT));
	SM_ErrorForward(relscan.GetNextRec(rec));
	SM_ErrorForward(relscan.CloseScan());
	return OK_RC;
}

RC SM_Manager::getAttrInfo(const char* relName, const char* attrName,
	DataAttrInfo &dinfo, RM_Record &rec) {
	RC WARN = SM_ATTRIBUTE_NOT_FOUND, ERR = SM_ATTRIBUTE_NOT_FOUND;
	RM_FileScan attrscan;
	SM_ErrorForward(attrscan.OpenScan(attrcat, STRING, MAXNAME + 1, 0, EQ_OP, (void*)relName, NO_HINT));
	bool found = false;
	while (attrscan.GetNextRec(rec) == OK_RC) {
		char* dinfodata;
		SM_ErrorForward(rec.GetData(dinfodata));
		memcpy(&dinfo, dinfodata, sizeof(DataAttrInfo));
		if (strcmp(dinfo.attrName, attrName) == 0) {
			found = true;
			break;
		}
	}
	SM_ErrorForward(attrscan.CloseScan());
	if (!found) return WARN;
	return OK_RC;
}

RC SM_Manager::getAttributes(const char *relName, vector<DataAttrInfo> &attributes) {
	RC WARN = SM_ATTRIBUTE_NOT_FOUND, ERR = SM_ATTRIBUTE_NOT_FOUND;
	RM_FileScan attrscan;
	RM_Record rec;
	SM_ErrorForward(attrscan.OpenScan(attrcat, STRING, MAXNAME + 1, 0, EQ_OP, (void*)relName, NO_HINT));
	DataAttrInfo dinfo;
	char *dinfodata;
	bool found = false;
	while (attrscan.GetNextRec(rec) == OK_RC) {
		found = true;
		SM_ErrorForward(rec.GetData(dinfodata));
		memcpy(&dinfo, dinfodata, sizeof(DataAttrInfo));
		attributes.push_back(dinfo);
	}
	SM_ErrorForward(attrscan.CloseScan());
	if (!found) return WARN;
	sort(attributes.begin(), attributes.end());
	return OK_RC;
}

RC SM_Manager::getRelation(const char* relName, RelationInfo &relation) {
	RC WARN = SM_RELATION_NOT_FOUND, ERR = SM_RELATION_NOT_FOUND;
	RM_Record rec;
	SM_ErrorForward(getRelInfo(relName, rec));
	char *relinfodata;
	SM_ErrorForward(rec.GetData(relinfodata));
	memcpy(&relation, relinfodata, sizeof(RelationInfo));
	return OK_RC;
}
