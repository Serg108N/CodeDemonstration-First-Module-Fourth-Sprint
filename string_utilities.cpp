#include "string_utilities.h"

#include <iostream>

using namespace std;

vector<string> SplitIntoWords(const string& text) {
	vector<string> words;
	string word;

	for (const char c : text) {
		if (c == ' ') {
			if (!word.empty())
				words.push_back(word);
			word.clear();
		}
		else {
			word += c;
		}
	}
	words.push_back(word);

	return words;
}

string ReadLine() {
	string s;
	getline(cin, s);

	return s;
}

int ReadLineWithNumber() {
	int result;
	cin >> result;
	ReadLine();

	return result;
}




