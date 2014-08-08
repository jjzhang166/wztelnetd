/*
 * Properties.cpp
 *
 *  Created on: 2012-7-7
 *      Author: zhangbo
 */

#include "Properties.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdexcept>

using namespace std;
#define MAX_LINE_LENGTH 4096

Properties::Properties() {
}

Properties::~Properties() {
}

string Properties::Get(const string& name, const string& def) {
	string ret;
	try {
		ret = this->properties[name];
	} catch (out_of_range& e) {
		ret = def;
	}
	return ret;
}

string Properties::GetString(const string& name, const string& def) {
	return this->Get(name, def);
}

void Properties::PutString(const string& name, const string& value) {
	this->Put(name, value);
}

unsigned long Properties::GetUnsignedLong(const string& name,
		unsigned long def) {
	string ret;
	try {
		ret = this->properties[name];
	} catch (out_of_range& e) {
		return def;
	}
	return strtoul(ret.c_str(), NULL, 10);
}

int Properties::GetInteger(const string& name, int def) {
	string ret;
	try {
		ret = this->properties[name];
	} catch (out_of_range& e) {
		return def;
	}
	return atoi(ret.c_str());
}

void Properties::PutInteger(const string& name, int value) {
	char str[1024];
	sprintf(str, "%d", value);
	this->properties[name] = str;
}

void Properties::Put(const string& name, const string& value) {
	this->properties[name] = value;
}

bool Properties::GetBoolean(const string& name, bool def) {
	bool ret = false;
	try {
		if (this->properties[name] == "true") {
			ret = true;
		}
	} catch (out_of_range& e) {
		return def;
	}
	return ret;
}

void Properties::PutBoolean(const string& name, bool value) {
	if (value) {
		this->properties[name] = "true";
	} else {
		this->properties[name] = "false";
	}
}

void Properties::Clear() {
	this->properties.clear();
}

map<string, string>::iterator Properties::Begin() {
	return this->properties.begin();
}

map<string, string>::iterator Properties::End() {
	return this->properties.end();
}

Properties Properties::Filter(const string& prefix) {
	Properties prop;
	map<string, string>::iterator it;
	for (it = properties.begin(); it != properties.end(); it++) {
		if (it->first.substr(0, prefix.length()) == prefix) {
			prop.Put(it->first.substr(prefix.length()), it->second);
		}
	}
	return prop;
}

bool Properties::Save(const string& file) {
	map<string, string>::iterator it;
	ofstream outf(file.c_str(), ios_base::out);
	for (it = properties.begin(); it != properties.end(); it++) {
		outf << it->first << "=" << it->second << endl;
	}
	outf.close();
	return true;
}

bool Properties::Load(const string& file) {
	ifstream inf(file.c_str(), ios_base::in);
	bool ret = this->Load(inf);
	inf.close();
	return ret;
}

bool Properties::Load(istream& inf) {
	this->Clear();
	char buf[MAX_LINE_LENGTH];
	inf.getline(buf, MAX_LINE_LENGTH);
	while (inf.good()) {
		string line = buf;
		if (line.length() > 0 && line.at(0) != '#') {
			string::size_type pos = line.find('=');
			if (pos != string::npos) {
				string name = Trim(line.substr(0, pos));
				string value = Trim(line.substr(pos + 1));
				properties[name] = value;
			}
		}
		inf.getline(buf, MAX_LINE_LENGTH);
	}
	return true;
}

bool Properties::Load(void* data, unsigned long size) {
	stringstream ss;
	ss.write((char*) data, size);
	return this->Load(ss);
}

string Properties::Trim(string str) {
	char chr;
	string::size_type i, j;
	if (str.length() == 0) {
		return str;
	}
	for (i = 0; i < str.length(); i++) {
		chr = str.at(i);
		if (chr == '\t' || chr == ' ' || chr == '\r' || chr == '\n') {
			continue;
		}
		break;
	}
	for (j = str.length() - 1; j >= i; j--) {
		chr = str.at(j);
		if (chr == '\t' || chr == ' ' || chr == '\r' || chr == '\n') {
			continue;
		}
		break;
	}
	str = str.substr(i, j - i + 1);
	return str;
}

bool Properties::LoadTable(const string& file) {
	ifstream inf(file.c_str(), ios_base::in);
	bool ret = this->LoadTable(inf);
	inf.close();
	return ret;
}

bool Properties::LoadTable(istream& inf) {
	this->Clear();
	char buf[MAX_LINE_LENGTH];
	inf.getline(buf, MAX_LINE_LENGTH);
	while (inf.good()) {
		string line = buf;
		line = Trim(line);
		if (line.length() > 0 && line.at(0) != '#') {
			string name = Shift(line);
			string value = Shift(line);
			properties[name] = value;
		}
		inf.getline(buf, MAX_LINE_LENGTH);
	}
	return true;
}

string Properties::Shift(string& line) {
	string::size_type pos = line.find('\t');
	if (pos != string::npos) {
		string name = Trim(line.substr(0, pos));
		line = Trim(line.substr(pos + 1));
		return name;
	}

	string ret = line;
	line = "";
	return ret;
}

bool Properties::LoadTable(void* data, unsigned long size) {
	stringstream ss;
	ss.write((char*) data, size);
	return this->LoadTable(ss);
}
