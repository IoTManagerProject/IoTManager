#pragma once

/*******************************************************************************
 **                                                                            **
 **                УДАЛИТЬ ЭТО ПРОСТО ПРИМЕР БИБЛИОТЕКИ                        **
 **                                                                            **
 *******************************************************************************/

class ExternalLibrary
{
private:
	/* data */
public:
	ExternalLibrary(/* args */);

	~ExternalLibrary();
	void begin (int pin);
	float READ_LIB_DATA_OTHER();
};

ExternalLibrary::ExternalLibrary(/* args */)
{
}

ExternalLibrary::~ExternalLibrary()
{
}

void ExternalLibrary::begin(int pin)
{

}

float ExternalLibrary::READ_LIB_DATA_OTHER()
{
	static float f = 0;
	return f++;
}