#include "search_server.h"
#include "string_utilities.h"

#include <stdexcept>
#include <cmath>

using namespace std;

Document::Document(int input_id, double input_relevance, int input_rating) {
	id = input_id;
	relevance = input_relevance;
	rating = input_rating;
}

SearchServer::SearchServer(const string& line_with_stop_words) {
	IsValidWord(line_with_stop_words);

	for (const string& word : SplitIntoWords(line_with_stop_words)) {
		stop_words_.insert(word);
	}
}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
	if (document_id < 0)
		throw invalid_argument("Document_id is negative"s);
	if (documents_.find(document_id) != documents_.end())
		throw invalid_argument("Document_id is already used"s);
	IsValidWord(document);

	const vector<string> words = SplitIntoWordsNoStop(document);
	const double inv_word_count = 1.0 / words.size();

	for (const string& word : words) {
		word_to_document_freqs_[word][document_id] += inv_word_count;
	}
	documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
	documents_vector_.push_back(document_id);
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
	IsValidWord(raw_query);
	const Query query = ParseQuery(raw_query);
	AreStopWordsValid(query);

	vector<string> matched_words;

	for (const string& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
			matched_words.push_back(word);
		}
	}

	for (const string& word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
			matched_words.clear();
			break;
		}
	}

	return {matched_words, documents_.at(document_id).status};
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus GetStatus) const {
	return FindTopDocuments(raw_query, [GetStatus](int document_id, DocumentStatus status, int rating) {
		return status == GetStatus;
	});
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
	if (ratings.empty())
		return 0;

	int rating_sum = 0;
	for (const int rating : ratings) {
		rating_sum += rating;
	}

	return rating_sum / static_cast<int>(ratings.size());
}

void SearchServer::IsValidWord(const string& word) {
	// A valid word must not contain special characters
	if (!none_of(word.begin(), word.end(), [](char c) {
		return c >= '\0' && c < ' ';
	})
		)
		throw invalid_argument("Invalid symbols in a line"s);
}

void SearchServer::AreStopWordsValid(const Query& query) {
	for (const auto& minus_word : query.minus_words) {
		if (minus_word.length() == 0)
			throw invalid_argument("A minus is not followed by a letter"s);
		if (minus_word[0] == '-')
			throw invalid_argument("Two minuses in a raw"s);
	}
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
	vector<string> words;

	for (const string& word : SplitIntoWords(text)) {
		if (!IsStopWord(word)) {
			words.push_back(word);
		}
	}

	return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
	bool is_minus = false;
	// Word shouldn't be empty
	if (text[0] == '-') {
		is_minus = true;
		text = text.substr(1);
	}

	return {
		text,
		is_minus,
		IsStopWord(text)
	};
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
	Query query;

	for (const string& word : SplitIntoWords(text)) {
		const QueryWord query_word = ParseQueryWord(word);

		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				query.minus_words.insert(query_word.data);
			}
			else {
				query.plus_words.insert(query_word.data);
			}
		}
	}

	return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
	return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
