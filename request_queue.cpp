#include "request_queue.h"

using namespace std;

//Я бы вообще все перенс в хэдер. Можете контр аргументировать.
//Реализацию методов в одну строку лучше располагть в h (хэдер) файлах, т.к. они будут встраиваемыми
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

