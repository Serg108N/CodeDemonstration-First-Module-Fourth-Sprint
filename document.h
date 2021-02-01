#pragma once

//В C++ в отличие от других языков, нормально, когда h или cpp файлы содержат не только исходный класс но и вспомогательные структуры и перечесления, а иногда и 
// другие классы.

//Эта струтура и перечесления, являются объектами которые использует SearchServer, я бы разместил их в том же хэдере.

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
