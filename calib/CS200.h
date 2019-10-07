#pragma once

#include <string>

class CS200 {
public:
	CS200();
	~CS200();
	bool open(int device_number = 0);
	int measure_start();
	bool get_result(std::string& result);
	void close();

	bool is_open() const;
	bool is_measuring() const;
private:
	bool m_is_open;
	bool m_is_measuring;
	int device_number;
};