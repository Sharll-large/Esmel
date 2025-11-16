//
// Created by Sharll on 2025/11/2.
//

#pragma once
#include <stdexcept>

class QuoteParseException : public std::runtime_error
{
public:
	QuoteParseException(const std::string &message, int line_num, int pos)
		: std::runtime_error(message), line_num_(line_num), pos_(pos)
	{
	}

	int getLineNumber() const { return line_num_; }
	int getPosition() const { return pos_; }

private:
	int line_num_;
	int pos_;
};
