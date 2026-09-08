#include "MessageXmlWriterTest.h"

#include <app/Message.h>
#include <interface/GraphicsDefs.h>
#include <interface/Rect.h>
#include <support/DataIO.h>
#include <support/String.h>

#include <math.h>
#include <stdio.h>

#include "MessageXmlWriter.h"
#include "ProjectConceptorDefs.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MessageXmlWriterTest);

void MessageXmlWriterTest::ZeroLengthRawFieldDoesNotCrash(void)
{
	// default: branch's char *encoded = new char[(size*2)] is a zero-size
	// allocation when size==0, immediately written to via encoded[len]='\0'
	// - a heap overflow either way, and a candidate for the reported SIGFPE
	// if encode_base64() itself divides by something size-derived.
	BMessage	message;
	message.AddData("Empty",B_RAW_TYPE,"",0);

	MessageXmlWriter	writer;
	BMallocIO			destination;
	CPPUNIT_ASSERT(writer.WriteTo(message,&destination) == B_OK);
}

void MessageXmlWriterTest::NaNFloatFieldDoesNotCrash(void)
{
	BMessage	message;
	message.AddFloat("NotANumber",NAN);

	MessageXmlWriter	writer;
	BMallocIO			destination;
	CPPUNIT_ASSERT(writer.WriteTo(message,&destination) == B_OK);
}

void MessageXmlWriterTest::InfinityDoubleFieldDoesNotCrash(void)
{
	BMessage	message;
	message.AddDouble("Unbounded",INFINITY);

	MessageXmlWriter	writer;
	BMallocIO			destination;
	CPPUNIT_ASSERT(writer.WriteTo(message,&destination) == B_OK);
}

void MessageXmlWriterTest::RawFieldSizesZeroToThreeHundredDoNotCrash(void)
{
	// broad fuzz across the default: branch's real dependency on size -
	// encode_base64()'s own source isn't available locally to read, and
	// #114's reported crash was a deterministic SIGFPE, so some specific
	// size (or size range, e.g. a base64 line-wrap boundary -
	// BASE64_LINELENGTH is 76 in the Haiku header) is the most likely
	// trigger. fprintf+fflush before each attempt: if this does crash, the
	// last printed size is the exact one, without needing to bisect again.
	char	buffer[300];
	for (int i=0; i<300; i++)
		buffer[i]	= (char)(i & 0xFF);

	for (ssize_t size=0; size<=300; size++) {
		fprintf(stderr,"MessageXmlWriterTest: trying raw size %ld\n",(long)size);
		fflush(stderr);

		BMessage	message;
		message.AddData("Raw",B_RAW_TYPE,buffer,size);

		MessageXmlWriter	writer;
		BMallocIO			destination;
		CPPUNIT_ASSERT(writer.WriteTo(message,&destination) == B_OK);
	}
}

void MessageXmlWriterTest::RealisticNestedNodeMessageDoesNotCrash(void)
{
	// #114's own investigation couldn't confirm whether the crash came
	// from ConfigManager::SaveConfig() or an actual document export via
	// the "ProjectConceptor Text" translator - a synthetic single-field
	// message might just not resemble either closely enough. This mirrors
	// a real node's actual shape (font/pattern sub-messages, B_INT8_TYPE
	// spacing/encoding, nested B_MESSAGE_TYPE, a B_POINTER_TYPE parent
	// reference, an rgb_color packed as int32 - see GenerateStressFixture.
	// cpp's NewNode(), verified against a real node dumped from the
	// running app) and pushes it through the exact same recursive
	// B_MESSAGE_TYPE path a real document's allNodes list goes through.
	BMessage	font('fOTy');
	font.AddInt8("Font::Encoding",0);
	font.AddInt16("Font::Face",0x40);
	font.AddString("Font::Family","Noto Sans");
	font.AddInt32("Font::Flags",0);
	font.AddFloat("Font::Rotation",0.0);
	font.AddFloat("Font::Shear",90.0);
	font.AddFloat("Font::Size",12.0);
	font.AddInt8("Font::Spacing",2);
	font.AddString("Font::Style","Regular");
	font.AddInt32("Font::Color",(int32)0xffb5976f);

	BMessage	pattern;
	rgb_color	fillColor	= {152,180,190,255};
	pattern.AddInt32("FillColor",*(int32*)&fillColor);
	pattern.AddFloat("PenSize",1.0);
	pattern.AddInt8("DrawingMode",B_OP_ALPHA);

	BMessage	data;
	data.AddString("Node::name","Node 1");

	BMessage	node(P_C_CLASS_TYPE);
	node.AddMessage("Node::Data",&data);
	node.AddRect("Node::Frame",BRect(0,0,100,40));
	node.AddMessage("Node::Font",&font);
	node.AddMessage("Node::Pattern",&pattern);
	node.AddBool("Node::selected",false);
	node.AddPointer("Node::parent",(void *)0x12345678);

	MessageXmlWriter	writer;
	BMallocIO			destination;
	CPPUNIT_ASSERT(writer.WriteTo(node,&destination) == B_OK);
}

void MessageXmlWriterTest::OrdinaryMessageRoundtrips(void)
{
	// sanity check: the fields the app actually writes on every save/quit
	// (ConfigManager::SaveConfig()) still work after whatever #114 fix
	// lands here
	BMessage	message;
	message.AddInt32("Count",42);
	message.AddString("Name","test");
	message.AddBool("Flag",true);
	message.AddRect("Frame",BRect(0,0,100,80));

	MessageXmlWriter	writer;
	BMallocIO			destination;
	CPPUNIT_ASSERT(writer.WriteTo(message,&destination) == B_OK);
	CPPUNIT_ASSERT(destination.BufferLength() > 0);
}
