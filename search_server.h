#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <tuple>

struct Document {
	Document() = default;
	Document(int input_id, double input_relevance, int input_rating);

	int id = 0;
	double relevance = 0;
	int rating = 0;
};

enum class DocumentStatus {
	ACTUAL, IRRELEVANT, BANNED, REMOVED
};

class SearchServer {

public:

	SearchServer(const std::string& line_with_stop_words);

	template<typename StringCollection>
	explicit SearchServer(const StringCollection& stop_words);

	void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

	int GetDocumentCount() const { return documents_.size(); }

	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

	int GetDocumentId(int index) const { return documents_vector_.at(index); }

	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus GetStatus) const;

	std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

	template<typename Predicate>
	std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicate predicate) const;

private:

	struct DocumentData {
		int rating;
		DocumentStatus status;
	};

	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};

	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};

	const std::size_t max_result_document_count = 5;
	std::set<std::string> stop_words_;
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	std::map<int, DocumentData> documents_;
	std::vector<int> documents_vector_;

	static int ComputeAverageRating(const std::vector<int>& ratings);

	static void IsValidWord(const std::string& word);

	static void AreStopWordsValid(const Query& query);

	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

	bool IsStopWord(const std::string& word) const { return stop_words_.count(word) > 0; }

	QueryWord ParseQueryWord(std::string text) const;

	Query ParseQuery(const std::string& text) const;

	double ComputeWordInverseDocumentFreq(const std::string& word) const;

	template<typename Predicate>
	std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const;
};

template<typename StringCollection>
SearchServer::SearchServer(const StringCollection& stop_words) {
	for (const std::string& word : stop_words) {
		IsValidWord(word);
	}

	for (const std::string& word : stop_words) {
		if (!word.empty())
		stop_words_.insert(word);
	}
}

template<typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, Predicate predicate) const {
	IsValidWord(raw_query);
	const Query query = ParseQuery(raw_query);
	AreStopWordsValid(query);

	auto matched_documents = FindAllDocuments(query, predicate);

	std::sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
		if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
			return lhs.rating > rhs.rating;
		}
		else {
			return lhs.relevance > rhs.relevance;
		}
	});

	if (matched_documents.size() > max_result_document_count) {
		matched_documents.resize(max_result_document_count);
	}

	return matched_documents;
}

template<typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Predicate predicate) const {
	std::map<int, double> document_to_relevance;

	for (const std::string& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0)
			continue;

		const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);

		for (const auto [ document_id, term_freq ] : word_to_document_freqs_.at(word)) {
			if (predicate(document_id, documents_.at(document_id).status, documents_.at(document_id).rating) == true) {
				document_to_relevance[document_id] += term_freq * inverse_document_freq;
			}
		}
	}

	for (const std::string& word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}

		for (const auto& [ document_id, _ ] : word_to_document_freqs_.at(word)) {
			document_to_relevance.erase(document_id);
		}
	}

	std::vector<Document> matched_documents;
	for (const auto& [ document_id, relevance ] : document_to_relevance) {
		matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
	}

	return matched_documents;
}
