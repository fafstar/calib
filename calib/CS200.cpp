#include "CS200.h"

#include <string>
#include <Windows.h>


HINSTANCE h = LoadLibrary(L"Kmsecs200.dll");

typedef INT(CALLBACK* USB_INI)(INT);
typedef INT(CALLBACK* USB_NUM)(VOID);
typedef INT(CALLBACK* USB_IO)(INT, LPSTR, INT, INT);

// load CS200 
USB_INI int_usb = (USB_INI)GetProcAddress(h, "int_usb");
USB_INI end_usb = (USB_INI)GetProcAddress(h, "end_usb");
USB_NUM get_num = (USB_NUM)GetProcAddress(h, "get_num");
USB_IO write64_usb = (USB_IO)GetProcAddress(h, "write64_usb");
USB_IO write16_usb = (USB_IO)GetProcAddress(h, "write16_usb");
USB_IO read64_usb = (USB_IO)GetProcAddress(h, "read64_usb");
USB_IO read16_usb = (USB_IO)GetProcAddress(h, "read16_usb");


CS200::CS200()
	: m_is_open(false), m_is_measuring(false)
{

}

CS200::~CS200() {
	close();
}

bool CS200::open(int dev_num) {

	int connected_count = get_num();

	if (!m_is_open && dev_num < connected_count) {
		// open usb pipeline
		int usb_id = int_usb(dev_num);
		if (usb_id == 0) {
			m_is_open = true;
			device_number = dev_num;
			// send control message
			char cmsg[] = "RMT,1\r\n";
			char readbuf[250];
			write64_usb(device_number, cmsg, 1, 7);
			read64_usb(device_number, readbuf, 1, 250);
		}
	}
	
	return false;
}

int CS200::measure_start() {
	if (m_is_open && !m_is_measuring) {
		m_is_measuring = true;
		// send control message
		char cmsg[] =  "MES,1\r\n";
		char readbuf[250];
		write64_usb(device_number, cmsg, 1, 7);
		read64_usb(device_number, readbuf, 1, 250);
		// get measuring duration
		std::string str = readbuf;
		int mes_time = std::stoi(str.substr(5, 2));
		// return measuring duration
		return mes_time;
	}
	return -1;
}

bool CS200::get_result(std::string & result) {
	if (m_is_measuring) {
		// send controll message
		char cmsg[] = "MDR,0\r\n";
		char readbuf[250];
		write64_usb(device_number, cmsg, 1, 7);
		read64_usb(device_number, readbuf, 1, 250);
		std::string sbuf = readbuf;
		// still measuring
		if (sbuf.substr(0, 4) == "ER02") {
			return false;
		}
		m_is_measuring = false;
		result = sbuf;
		return true;
	}

	return false;
}

void CS200::close() {
	if (m_is_open && !m_is_measuring) {
		// send controll message
		char cmsg[] = "RMT,0\r\n";
		char readbuf[250];
		write64_usb(device_number, cmsg, 1, 7);
		read64_usb(device_number, readbuf, 1, 250);
		// close usb pipeline
		end_usb(device_number);
		m_is_open = false;
	}
}

bool CS200::is_open() const
{
	return m_is_open;
}

bool CS200::is_measuring() const
{
	return m_is_measuring;
}

