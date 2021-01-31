#pragma once

#include "search_server.h"
#include "document.h"

#include <string>
#include <deque>
#include <vector>

class RequestQueue {

public:

	explicit RequestQueue(const SearchServer& search_server);

	template<typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
		++current_time_;
		if (current_time_ > sec_in_day_) {
			if (!requests_.front().IsActual)
				--number_of_noresult_requests;
			requests_.pop_front();
			--current_time_;
		}

		QueryResult current_request;
		current_request.documents_ = server_.FindTopDocuments(raw_query, document_predicate);
		if (current_request.documents_.empty()) {
			current_request.IsActual = false;
			++number_of_noresult_requests;
		}
		else {
			current_request.IsActual = true;
		}
		requests_.push_back(current_request);

		return current_request.documents_;
	}

	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus GetStatus);

	std::vector<Document> AddFindRequest(const std::string& raw_query);

	int GetNoResultRequests() const;

private:

	struct QueryResult {
		bool IsActual;
		std::vector<Document> documents_;
	};

	int number_of_noresult_requests;
	int current_time_;
	const static int sec_in_day_ = 1440;
	std::deque<QueryResult> requests_;
	const SearchServer& server_;
};
