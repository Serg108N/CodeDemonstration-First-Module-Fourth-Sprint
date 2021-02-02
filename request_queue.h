#pragma once

#include "search_server.h"

#include <string>
#include <deque>
#include <vector>

class RequestQueue {

public:

	explicit RequestQueue(const SearchServer& search_server) : number_of_noresult_requests(0), current_time_(0), server_(search_server) {}

	template<typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus GetStatus) {
		return AddFindRequest(raw_query, [GetStatus](int document_id, DocumentStatus status, int rating) {
			return status == GetStatus;
		});
	}

	std::vector<Document> AddFindRequest(const std::string& raw_query) { return AddFindRequest(raw_query, DocumentStatus::ACTUAL); }

	int GetNoResultRequests() const { return number_of_noresult_requests; }

private:

	struct QueryResult {
		bool IsActual;
		std::vector<Document> documents_;
	};

	const static int sec_in_day_ = 1440;

	int number_of_noresult_requests;
	int current_time_;
	std::deque<QueryResult> requests_;
	const SearchServer& server_;
};

template<typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
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
