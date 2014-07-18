/*
 * Properties.h
 *
 *  Created on: 2012-7-7
 *      Author: zhangbo
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <string>
#include <map>
#include <vector>
using namespace std;

class Properties {
private:
	map<string, string> properties;

public:
	Properties();
	virtual ~Properties();

	string Get(const string& name, const string& def = "");
	void Put(const string& name, const string& value);

	string GetString(const string& name, const string& def = "");
	void PutString(const string& name, const string& value);

	int GetInteger(const string& name, int def = 0);
	void PutInteger(const string& name, int value);

	bool GetBoolean(const string& name, bool def = false);
	void PutBoolean(const string& name, bool value);

	double GetDouble(const string& name, double def = 0.0);
	void PutDouble(const string& name, double value);

	unsigned long GetUnsignedLong(const string& name, unsigned long def = 0);
	void PutUnsignedLong(const string& name, unsigned long value);

	vector<string> GetStrings(const string& name, char c);

	void Clear();

	map<string, string>::iterator Begin();

	map<string, string>::iterator End();

	bool Save(const string& file);

	bool Load(const string& file);

	bool Load(istream& inf);

	bool Load(void* data, unsigned long size);

	bool LoadTable(const string& file);

	bool LoadTable(istream& inf);

	bool LoadTable(void* data, unsigned long size);

	Properties Filter(const string& prefix);

	string Trim(string str);

	string Shift(string& str);
};

#endif /* PROPERTIES_H_ */
