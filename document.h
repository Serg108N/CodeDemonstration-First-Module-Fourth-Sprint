#pragma once

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
