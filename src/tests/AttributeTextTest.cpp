#include "AttributeTextTest.h"

#include <interface/GraphicsDefs.h>

#include "AttributeText.h"
#include "ProjectConceptorDefs.h"


void AttributeTextTest::ValueTextOfEveryKind(void)
{
	BString		text;
	BMessage	note(P_C_ATTRIBUTE_NOTE);
	note.AddString(P_C_ATTRIBUTE_VALUE,"two\nlines");
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,AttributeValueText(note,&text));
	CPPUNIT_ASSERT(text == "two\nlines");

	BMessage	yes(B_BOOL_TYPE);
	yes.AddBool(P_C_ATTRIBUTE_VALUE,true);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,AttributeValueText(yes,&text));
	CPPUNIT_ASSERT(text == "true");

	BMessage	number(P_C_ATTRIBUTE_NUMBER);
	number.AddFloat(P_C_ATTRIBUTE_VALUE,25.5f);
	number.AddString(P_C_ATTRIBUTE_UNIT,"€");
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,AttributeValueText(number,&text));
	CPPUNIT_ASSERT(text == "25.5 €");

	BMessage	plain(P_C_ATTRIBUTE_NUMBER);
	plain.AddFloat(P_C_ATTRIBUTE_VALUE,3.0f);
	plain.AddString(P_C_ATTRIBUTE_UNIT,"");
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,AttributeValueText(plain,&text));
	CPPUNIT_ASSERT(text == "3");

	BMessage	color(P_C_ATTRIBUTE_COLOR);
	rgb_color	red	= {220,90,60,255};
	color.AddData(P_C_ATTRIBUTE_VALUE,B_RGB_COLOR_TYPE,&red,sizeof(red));
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,AttributeValueText(color,&text));
	CPPUNIT_ASSERT(text == "#dc5a3c");
}


void AttributeTextTest::NoValueIsAnError(void)
{
	BString		text("unchanged");
	BMessage	empty(P_C_ATTRIBUTE_LINK);
	empty.AddString(P_C_ATTRIBUTE_NAME,"Link");
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,AttributeValueText(empty,&text));
}


void AttributeTextTest::NumbersWithUnits(void)
{
	float	number	= 0;
	BString	unit;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("25,5 €",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(25.5,number,0.0001);
	CPPUNIT_ASSERT(unit == "€");
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("  -3h ",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(-3,number,0.0001);
	CPPUNIT_ASSERT(unit == "h");
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("42",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(42,number,0.0001);
	CPPUNIT_ASSERT(unit == "");
	// the space between number and unit is no digit group
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("7 days",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(7,number,0.0001);
	CPPUNIT_ASSERT(unit == "days");
}


void AttributeTextTest::SeparatorsDecideThePoint(void)
{
	float	number	= 0;
	BString	unit;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("1.234,5 km",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1234.5,number,0.01);
	CPPUNIT_ASSERT(unit == "km");
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("1,234.5",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1234.5,number,0.01);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("1.234.567",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1234567,number,1);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseNumberWithUnit("12 500 €",&number,&unit));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(12500,number,0.01);
	CPPUNIT_ASSERT(unit == "€");
}


void AttributeTextTest::TextWithoutNumberIsRefused(void)
{
	float	number	= 7;
	BString	unit("kept");
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,ParseNumberWithUnit("about 5",&number,&unit));
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,ParseNumberWithUnit("",&number,&unit));
	CPPUNIT_ASSERT_EQUAL(7.0f,number);
	CPPUNIT_ASSERT(unit == "kept");
}
