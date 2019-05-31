// Grammar.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stack>
#include<string>
#include <io.h>

#define _BEGIN 1
#define _END 2
#define _INTEGER 3
#define _IF 4
#define _THEN 5
#define _ELSE 6
#define _FUNCTION 7
#define _READ 8
#define _WRITE 9
#define _VAR 10
#define _CONST 11
#define _EQUAL 12
#define _NEQ 13
#define _LE 14
#define _LESS 15
#define _ME 16
#define _MORE 17
#define _SUB 18
#define _MUL 19
#define _IS 20
#define _LBR 21
#define _RBR 22
#define _SEM 23
#define EOLN 24
#define EOF 25

#define LACK 50
#define MATCH 51
#define DEF 52
#define NDEF 53

int line_count_error = 1;
int plv = 1;
int vadr = 0;
std::stack <std::string> pro_stack;

int Grammar(char * source, char * destination, char * var_list, char * pro_list, char * error);
char * Advance(FILE * source,FILE * destination);
int FindType(char * sym);
void SubPro(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void Error(FILE * error, int type, char * sym,char * match);
void DeclareList(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void ExecuteList(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void DeclareSentence(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void DeclareTemp(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void ExcuteSentence(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void ExcuteTemp(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void TwoDeclare(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym);
void Assign(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void ArithmeticExpression(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void ArithmeticTemp(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void Item(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void Factor(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void ItemTemp(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void FunctionCall(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void ConditionExpression(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym);
void RelationalOperator(FILE * source, FILE * destination, FILE * error, char * sym);
void AddVar(FILE * var_list, char * vname, char * vproc, int vkind, char * vtype, int vlv, int vadr);
void AddPro(FILE * pro_list, char * pname, char * ptype, int plv);
void BackpatchFormal(FILE * var_list, char * vname, char * vproc, char * vtype);
void BackpatchFun(FILE * pro_list, char * pname);
char * GetName(char * sym);
int VarRedefine(FILE * var_list,char * vname,char * vproc);
int ProRedefine(FILE * pro_list, char * pname);
int ReadAhead(FILE * source);

int main(){
	Grammar("test.dyd", "test.dys", "test.var", "test.pro", "test.err2");
	printf("End as line:%d\n", line_count_error);
	getchar();
    return 0;
}
int Grammar(char * source, char * destination, char * var_list, char * pro_list, char * error) {
	FILE *fps, *fpd, *fpe, *fpv, *fpp;
	fps = fopen(source, "r");
	fpd = fopen(destination, "wt+");
	fpe = fopen(error, "wt+");
	fpv = fopen(var_list, "wt+");
	fpp = fopen(pro_list, "wt+");

	char sym[20];
	pro_stack.push("            main");
	strcpy(sym,Advance(fps, fpd));
	SubPro(fps, fpd, fpv, fpp, fpe, sym);
	return 0;
}
char * Advance(FILE * source, FILE * destination) {
	char temp[21];
	if (!source) {
		return NULL;
	}
	do {
		fgets(temp, 21, source);
		temp[19] = '\0';
		fprintf(destination, "%s\n", temp);
		fflush(destination);
		int fd = _fileno(destination);
		_commit(fd);
		if (EOLN == FindType(temp)) {
			line_count_error++;
			continue;
		}
		break;
	} while (source);
	return temp;
}
int FindType(char * sym) {
	if (!sym) {
		return 0;
	}
	return (sym[17] - '0') * 10 + sym[18] - '0';
}
void SubPro(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//SubPro->begin DeclareList;ExecuteList end
	if (_BEGIN == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
	}
	else {
		Error(error, LACK, NULL, "begin");
	}
	DeclareList(source, destination, var_list, pro_list, error, sym);
	if (_SEM == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
	}
	else {
		Error(error, LACK, NULL, "\";\"");
	}
	ExecuteList(source, destination, var_list, pro_list, error, sym);
	if (_END == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
		return;
	}
	else {
		Error(error, LACK, NULL, "end");
		return;
	}
}
void Error(FILE * error, int type, char * sym, char * match) {
	char temp_DEF[17];
	switch (type) {
	case LACK:
		fprintf(error, "***LINE:%2d  symbol missing:%s\n", line_count_error, match);
		break;
	case DEF:
		strcpy(temp_DEF, GetName(sym));
		fprintf(error, "***LINE:%2d  symbol redefine:%s\n", line_count_error, temp_DEF);
		break;
	case NDEF:
		strcpy(temp_DEF, GetName(sym));
		fprintf(error, "***LINE:%2d  symbol undefined:%s\n", line_count_error, temp_DEF);
		break;
	case MATCH:
		fprintf(error, "***LINE:%2d  symbol unmatched:%s\n", line_count_error, match);
		break;
	default:
		;
	}
	fflush(error);
	int fd = _fileno(error);
	_commit(fd);
}
void DeclareList(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//DeclareList->DeclareSentence DeclareTemp
	DeclareSentence(source, destination, var_list, pro_list, error, sym);
	DeclareTemp(source, destination, var_list, pro_list, error, sym);
}
void ExecuteList(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//ExecuteList->ExcuteSentence ExcuteTemp
	ExcuteSentence(source,destination,var_list,pro_list,error,sym);
	ExcuteTemp(source,destination,var_list,pro_list,error,sym);
}
void DeclareSentence(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//DeclareSentence->integer TwoDeclare
	if (_INTEGER == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
		TwoDeclare(source, destination, var_list, pro_list, error, sym);
	}
	//VarDeclare(source,destination,var_list,error,sym);
	//FunDeclare(source, destination, var_list, pro_list, error, sym);
}
void DeclareTemp(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//DeclareTemp->;DeclareList|e
	if (_SEM == FindType(sym)&& ReadAhead(source)) {
		strcpy(sym, Advance(source, destination));
		DeclareList(source, destination, var_list, pro_list, error, sym);
	}
}
void ExcuteSentence(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//ExcuteSentence->read(_VAR)|write(_VAR)|Assign|if ConditionExpression then ExcuteSentence else ExcuteSentence
	char name[17];
	char top_name[17];
	switch (FindType(sym)) {
	case _READ:
	case _WRITE:
		strcpy(sym, Advance(source, destination));
		if (_LBR == FindType(sym)) {
			strcpy(sym, Advance(source, destination));
		}
		else {
			Error(error, LACK, sym, "\"(\"");
		}
		if (_VAR == FindType(sym)) {
			strncpy(name, GetName(sym), 17);
			strncpy(top_name, pro_stack.top().c_str(), 17);
			if (0 == VarRedefine(var_list, name, top_name)) {
				Error(error, NDEF, sym, NULL);
			}
			strcpy(sym, Advance(source, destination));
		}
		else {
			Error(error, LACK, sym, "variable");
		}
		if (_RBR == FindType(sym)) {
			strcpy(sym, Advance(source, destination));
		}
		else {
			Error(error, MATCH, sym, "\"(\"");
		}
		break;
	case _IF:
		strcpy(sym, Advance(source, destination));
		ConditionExpression(source, destination, var_list, pro_list, error, sym);
		if (_THEN == FindType(sym)) {
			strcpy(sym, Advance(source, destination));
		}
		else {
			Error(error, LACK, sym, "then");
		}
		ExcuteSentence(source, destination, var_list, pro_list, error, sym);
		if (_ELSE == FindType(sym)) {
			strcpy(sym, Advance(source, destination));
		}
		else {
			Error(error, MATCH, sym, "if");
		}
		ExcuteSentence(source, destination, var_list, pro_list, error, sym);
		break;
	default:
		Assign(source, destination, var_list, pro_list, error, sym);

	}
}
void ExcuteTemp(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//ExcuteTemp->;ExcuteList|e
	if (_SEM == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
		ExecuteList(source, destination, var_list, pro_list, error, sym);
	}
}
void TwoDeclare(FILE * source, FILE * destination, FILE * var_list, FILE * pro_list, FILE * error, char * sym) {
	//TwoDeclare->_VAR|function _VAR (_VAR) ; FunctionBody
	char top_name[17];
	char name[17];
	strncpy(top_name, pro_stack.top().c_str(),17);
	strncpy(name, GetName(sym), 17);
	top_name[16] = '\0';
	if (_VAR == FindType(sym)) {
		if (0==VarRedefine(var_list,name,top_name)) {
			AddVar(var_list,name ,top_name, 0, "int", plv, vadr);
			vadr++;
		}
		else if(2== VarRedefine(var_list, name, top_name)){
			BackpatchFormal(var_list, name, top_name, "int");
		}
		else {
			Error(error, DEF, sym, NULL);
		}
		strcpy(sym, Advance(source, destination));
	}
	else if (_FUNCTION == FindType(sym)) {
		plv++;
		strcpy(sym, Advance(source, destination));
		if (_VAR == FindType(sym)) {
			char temp_function_name[17];
			strcpy(temp_function_name, GetName(sym));
			pro_stack.push(temp_function_name);
			if (!ProRedefine(pro_list, temp_function_name)) {

				AddPro(pro_list, temp_function_name, "int", plv);
				//FILE * temp_ladr = pro_list;
				//fprintf();
				strcpy(sym, Advance(source, destination));
				if (_LBR == FindType(sym)) {
					strcpy(sym, Advance(source, destination));
				}
				else {
					Error(error, LACK, sym, "\"(\"");
				}
				if (_VAR == FindType(sym)) {
					strncpy(name, GetName(sym), 17);
					AddVar(var_list, name, temp_function_name, 2, "null", plv, vadr);
					strcpy(sym, Advance(source, destination));
					if (_RBR == FindType(sym)) {
						strcpy(sym, Advance(source, destination));
					}
					else {
						Error(error, MATCH, sym, "\"(\"");
					}
					if (_SEM == FindType(sym)) {
						strcpy(sym, Advance(source, destination));
					}
					else {
						Error(error, LACK, sym, "\";\"");
					}
					SubPro(source,destination,var_list,pro_list,error,sym);
				}
				//fprintf();
				BackpatchFun(pro_list, temp_function_name);
			}
			else {
				Error(error, DEF, sym, NULL);
			}
			
			pro_stack.pop();
		}
		else {
			Error(error, LACK, sym, "function name");
		}
		plv--;
	}
		  
}
void Assign(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//Assign-> _VAR := ArithmeticExpression
	char top_name[17];
	char name[17];
	strncpy(top_name, pro_stack.top().c_str(), 17);
	strncpy(name, GetName(sym), 17);

	if (_VAR == FindType(sym)) {
		if (2 == VarRedefine(var_list, name, top_name)) {
			Error(error, NDEF, sym, NULL);
		}
		else if (1 == VarRedefine(var_list, name, top_name)) {
			;
		}
		else if (!ProRedefine(pro_list, name)) {
			Error(error, NDEF, sym, NULL);
		}
		strcpy(sym, Advance(source, destination));
	}
	else if (_CONST == FindType(sym)) {
		Error(error, MATCH, sym, "\":=\"");
		strcpy(sym, Advance(source, destination));
	}
	else {
		Error(error, LACK, sym, "variable");
	}

	if (_IS == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
	}
	else {
		Error(error, LACK, sym, "\":=\"");
	}

	ArithmeticExpression(source,destination,var_list,pro_list,error,sym);

}
void ArithmeticExpression(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//ArithmeticExpression->item ArithmeticTemp
	Item(source, destination, var_list, pro_list, error, sym);
	ArithmeticTemp(source, destination, var_list, pro_list, error, sym);
}
void ArithmeticTemp(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//ArithmeticTemp->-ArithmeticExpression|e
	if (_SUB == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
		ArithmeticExpression(source, destination, var_list, pro_list, error, sym);
	}
}
void Item(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//Item->Factor ItemTemp
	Factor(source, destination, var_list, pro_list, error, sym);
	ItemTemp(source, destination, var_list, pro_list, error, sym);
}
void Factor(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//Factor->_VAR|_CONST|FunctionCall
	if (_CONST == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
	}
	else if (_VAR == FindType(sym)) {
		char top_name[17];
		char name[17];
		strncpy(top_name, pro_stack.top().c_str(), 17);
		strncpy(name, GetName(sym), 17);
		if (1 == VarRedefine(var_list, name, top_name)) {
			strcpy(sym, Advance(source, destination));
			return;
		}
		else if (2 == VarRedefine(var_list, name, top_name)) {
			Error(error, NDEF, sym, NULL);
			return;
		}
		else{
			FunctionCall(source, destination, var_list, pro_list, error, sym);
		}
	}
	/*else {
		FunctionCall(source, destination, var_list, pro_list, error, sym);
	}*/
}
void ItemTemp(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//Item->* Item|e
	if (_MUL == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
		Item(source, destination, var_list, pro_list, error, sym);
	}
}
void FunctionCall(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//FunctionCall->_VAR(ArithmeticExpression)
	char top_name[17];
	char name[17];
	strncpy(top_name, pro_stack.top().c_str(), 17);
	strncpy(name, GetName(sym), 17);
	if (_VAR == FindType(sym)) {
		if (!ProRedefine(pro_list, name)) {
			Error(error, NDEF, sym, NULL);
		}
		strcpy(sym, Advance(source, destination));
	}
	else {
		Error(error, LACK, sym, "function name");
	}
	if (_LBR == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
	}
	else {
		Error(error, LACK, sym, "\"(\"");
	}
	ArithmeticExpression(source, destination, var_list, pro_list, error, sym);
	if (_RBR == FindType(sym)) {
		strcpy(sym, Advance(source, destination));
	}
	else {
		Error(error, MATCH, sym, "\"(\"");
	}
}
void ConditionExpression(FILE *source, FILE * destination, FILE * var_list, FILE * pro_list, FILE *error, char * sym) {
	//ConditionExpression->ArithmeticExpression RelationalOperator ArithmeticExpression
	ArithmeticExpression(source, destination, var_list, pro_list, error, sym);
	RelationalOperator(source, destination, error, sym);
	ArithmeticExpression(source, destination, var_list, pro_list, error, sym);
}
void RelationalOperator(FILE * source, FILE * destination, FILE * error, char * sym) {
	//RelationalOperator-><│<=│>│>=│=│<>
	switch (FindType(sym)) {
	case _EQUAL:
	case _LE:
	case _ME:
	case _LESS:
	case _MORE:
	case _NEQ:
		strcpy(sym, Advance(source, destination));
		break;
	default:
		Error(error, LACK, sym, "relational operator");
	}
}
void AddVar(FILE * var_list, char * vname, char * vproc, int vkind, char * vtype, int vlv, int vadr) {
	fseek(var_list, 0, SEEK_END);
	fprintf(var_list, "vname:%16s\n", vname);
	fprintf(var_list, "vproc:%16s\n", vproc);
	fprintf(var_list, "vkind:%16d\n", vkind);
	fprintf(var_list, "vtype:%16s\n", vtype);
	fprintf(var_list, "  vlv:%16d\n", vlv);
	fprintf(var_list, " vadr:%16d\n", vadr);
	fflush(var_list);
	int fd = _fileno(var_list);
	_commit(fd);
}
void AddPro(FILE * pro_list, char * pname, char * ptype, int plv){
	fseek(pro_list, 0, SEEK_END);
	fprintf(pro_list, "pname:%16s\n", pname);
	fprintf(pro_list, "ptype:%16s\n", ptype);
	fprintf(pro_list, "  plv:%16d\n", plv);
	fprintf(pro_list, " fadr:%16d\n", vadr);
	fprintf(pro_list, " ladr:%16d\n", 0);
	fflush(pro_list);
	int fd = _fileno(pro_list);
	_commit(fd);
}
char * GetName(char * sym) {
	char temp[17];
	strncpy(temp, sym, 16);
	temp[16] = '\0';
	return temp;
}
int VarRedefine(FILE * var_list, char * vname, char * vproc) {
	char temp_sym[24];
	fseek(var_list, 0, SEEK_SET);
	while (!feof(var_list)) {
		fgets(temp_sym, 24, var_list);
		temp_sym[22] = '\0';
		if (!strcmp(temp_sym + 6, vname)) {
			fgets(temp_sym, 24, var_list);
			temp_sym[22] = '\0';
			if (!strcmp(temp_sym + 6, vproc)) {
				fgets(temp_sym, 24, var_list);
				if ('2' == temp_sym[21]) {
					//Formal parameter which doesn't backpatch;
					return 2;
				}
				return 1;
			}
			//fseek(var_list, 24 * 3, SEEK_CUR);
			for (int i = 0; i < 3; i++) {
				fgets(temp_sym, 24, var_list);
			}
			continue;
		}
		//fseek(var_list, 24 * 5, SEEK_CUR);
		for (int i = 0; i < 5; i++) {
			fgets(temp_sym, 24, var_list);
		}
	}
	return 0;
}
int ProRedefine(FILE * pro_list, char * pname) {
	char temp_sym[24];
	fseek(pro_list, 0, SEEK_SET);
	while (!feof(pro_list)) {
		fgets(temp_sym, 24, pro_list);
		temp_sym[22] = '\0';
		if (!strcmp(temp_sym + 6, pname)) {
			return true;
		}
		//fseek(pro_list, 24 * 4, SEEK_CUR);
		for (int i = 0; i < 4; i++) {
			fgets(temp_sym, 24, pro_list);
		}
	}
	return false;
}
void BackpatchFormal(FILE * var_list, char * vname, char * vproc, char * vtype) {
	char temp_sym[24];
	fseek(var_list, 0, SEEK_SET);
	while (!feof(var_list)) {
		fgets(temp_sym, 24, var_list);
		temp_sym[22] = '\0';
		if (!strcmp(temp_sym + 6, vname)) {
			fgets(temp_sym, 24, var_list);
			temp_sym[22] = '\0';
			if (!strcmp(temp_sym + 6, vproc)) {
				fgets(temp_sym, 24, var_list);
				if ('2' == temp_sym[21]) {
					//Formal parameter which doesn't backpatch;
					fseek(var_list, -24, SEEK_CUR);
					fprintf(var_list, "vkind:%16d\n", 1);
					fprintf(var_list, "vtype:%16s\n", vtype);
					fflush(var_list);
					int fd = _fileno(var_list);
					_commit(fd);
					return;
					return;
				}
				return;
			}
			//fseek(var_list, 24 * 3, SEEK_CUR);
			for (int i = 0; i < 3; i++) {
				fgets(temp_sym, 24, var_list);
			}
			continue;
		}
		//fseek(var_list, 24 * 5, SEEK_CUR);
		
		for (int i = 0; i < 5; i++) {
			fgets(temp_sym, 24, var_list);
		}
	}
	return;
}
void BackpatchFun(FILE * pro_list, char * pname) {
	char temp_sym[24];
	fseek(pro_list, 0, SEEK_SET);
	while (!feof(pro_list)) {
		fgets(temp_sym, 24, pro_list);
		temp_sym[22] = '\0';
		if (!strcmp(temp_sym + 6, pname)) {
			for (int i = 0; i < 3; i++) {
				fgets(temp_sym, 24, pro_list);
			}
			//fseek(pro_list, 24 * 3, SEEK_CUR);
			fseek(pro_list, 0, SEEK_CUR);
			fprintf(pro_list, " ladr:%16d\n", vadr);
			fflush(pro_list);
			int fd = _fileno(pro_list);
			_commit(fd);
			return;
		}
		//fseek(pro_list, 24 * 4, SEEK_CUR);
		for (int i = 0; i < 4; i++) {
			//fgets(temp_sym, 24, pro_list);
			fgets(temp_sym, 24, pro_list);
		}
	}
	return;
}
int ReadAhead(FILE * source) {
	char temp[21];
	int flag = 1;
	if (!source) {
		return 0;
	}
	do {
		fgets(temp, 21, source);
		temp[19] = '\0';
		switch (FindType(temp)) {
		case EOLN:
			flag++;
			continue;
		case _INTEGER:
			fseek(source, -21 * flag, SEEK_CUR);
			return 1;
		default:
			fseek(source, -21 * flag, SEEK_CUR);
			return 0;
		}
		break;
	} while (source);
	return 0;
}