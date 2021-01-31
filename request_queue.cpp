#include "request_queue.h"

using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server) :
		number_of_noresult_requests(0), current_time_(0), server_(search_server) {
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus GetStatus) {
	return AddFindRequest(raw_query, [GetStatus](int document_id, DocumentStatus status, int rating) {
		return status == GetStatus;
	});
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
	return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
	return number_of_noresult_requests;
}

