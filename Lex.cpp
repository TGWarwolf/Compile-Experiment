// Lex.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<stdio.h>
#include<string.h>

int Tex(char * source, char * destination, char * error);
int IsLetter(char ch);
int IsNumber(char ch);
int WordType(char * name);
void Output(FILE * fp, char * name, int type);
void Error(FILE * fp, int line_count, char * type);
int main(){
	Tex("test.pas", "test.dyd", "test.err");
    return 0;
}
int Tex(char * source, char * destination, char * error) {
	FILE *fps,*fpd,*fpe;
	char ch;
	char word[17];
	int word_count = 0, line_count = 1;
	int back_flag = 0;
	fps = fopen(source, "r");
	fpd = fopen(destination, "w+");
	fpe = fopen(error, "w+");

	while (true) {
		word_count = 0;
		if (!back_flag) {
			ch = fgetc(fps);
		}	
		if (' ' == ch) {
			back_flag = 0;
		}
		else if (IsLetter(ch)) {
			while ((IsLetter(ch)||IsNumber(ch))&&word_count<16) {
				word[word_count] = ch;
				word_count++;
				ch = fgetc(fps);
			}
			word[word_count] = '\0';
			back_flag = 1;
			Output(fpd, word, WordType(word));
			continue;
		}
		else if (IsNumber(ch)) {
			while (IsNumber(ch) && word_count<16) {
				word[word_count] = ch;
				word_count++;
				ch = fgetc(fps);
			}
			word[word_count] = '\0';
			back_flag = 1;
			Output(fpd, word, 11);
			continue;
		}
		else {
			switch (ch) {
			case '=':
				Output(fpd, "=", 12);
				break;
			case '<':
				ch = fgetc(fps);
				if ('>' == ch) {
					Output(fpd, "<>", 13);
				}
				else if ('=' == ch) {
					Output(fpd, "<=", 14);
				}
				else {
					Output(fpd, "<", 15);
					back_flag = 1;
					continue;
				}
				break;
			case '>':
				ch = fgetc(fps);
				if ('=' == ch) {
					Output(fpd, ">=", 16);
				}
				else {
					Output(fpd, ">", 17);
					back_flag = 1;
					continue;
				}
				break;
			case '-':
				Output(fpd, "-", 18);
				break;
			case '*':
				Output(fpd, "*", 19);
				break;
			case ':':
				ch = fgetc(fps);
				if ('=' == ch) {
					Output(fpd, ":=", 20);
				}
				else {
					Error(fpe, line_count, "\':\' not match");
					back_flag = 1;
					continue;
				}
				break;
			case '(':
				Output(fpd, "(", 21);
				break;
			case ')':
				Output(fpd, ")", 22);
				break;
			case ';':
				Output(fpd, ";", 23);
				break;
			case '\n':
				Output(fpd, "EOLN", 24);
				line_count++;
				break;
			case EOF:
				Output(fpd, "EOF", 25);
				fclose(fps);
				fclose(fpd);
				fclose(fpe);
				return 1;
			default:
				Error(fpe, line_count, "illegal character");
			}
		}
		back_flag = 0;
	}
	
	fclose(fps);
	fclose(fpd);
	fclose(fpe);
	return 0;
}
int IsLetter(char ch) {
	if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A'&&ch <= 'Z'))return true;
	return false;
}
int IsNumber(char ch) {
	if (ch >= '0'&&ch <= '9')return true;
	return false;
}
void Output(FILE * fp, char * name, int type) {
	fprintf(fp, "%16s ", name);
	if (type < 10)fprintf(fp, "0");
	fprintf(fp, "%d\n", type);
}
void Error(FILE * fp, int line_count, char * type) {
	fprintf(fp, "***LINE:%d  %s\n", line_count, type);
}
int WordType(char * name) {
	if (!strcmp(name, "begin"))return 1;
	if (!strcmp(name, "end")) return 2;
	if (!strcmp(name, "integer"))return 3;
	if (!strcmp(name, "if"))return 4;
	if (!strcmp(name, "then"))return 5;
	if (!strcmp(name, "else"))return 6;
	if (!strcmp(name, "function"))return 7;
	if (!strcmp(name, "read"))return 8;
	if (!strcmp(name, "write"))return 9;
	return 10;
}

