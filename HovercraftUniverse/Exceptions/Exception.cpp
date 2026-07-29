#include "Exception.h"

#include <sstream>

namespace HovUni {

Exception::Exception(const std::string& msg) : mMsg(msg), mNested(0), mFilename("") {

}

Exception::Exception(const std::string& msg, const std::string& filename, unsigned line) : 
	mMsg(msg), mNested(0), mFilename(filename), mLineNumber(line) {
		
}

Exception::Exception(const std::string& msg, const Exception& nested) : mMsg(msg), mNested(nested.clone()), mFilename("") {

}

Exception::Exception(const std::string& msg, const Exception& nested, const std::string& filename, unsigned line) : mMsg(msg), mNested(nested.clone()), mFilename(filename), mLineNumber(line) {

}
	
Exception::Exception(const Exception& ex) : mMsg(ex.mMsg), mNested(0), mFilename(ex.mFilename), mLineNumber(ex.mLineNumber) {
	mNested = (ex.mNested ? ex.mNested->clone() : 0);
}
	
Exception::~Exception() throw() {
	delete mNested;
	mNested = 0;
}

Exception& Exception::operator=(const Exception& ex) {
	if (&ex != this) {
		delete mNested;
		mMsg = ex.mMsg;
		mNested = (ex.mNested ? ex.mNested->clone() : 0);
		mFilename = ex.mFilename;
		mLineNumber = ex.mLineNumber;
	}
	return *this;
}

Exception* Exception::clone() const {
	return new Exception(*this);
}

std::string Exception::getMessage() const {
	std::ostringstream  msg;
	
	msg << className() << " : " << mMsg;
	
	if (mFilename != "") {
		msg << " [FILE: " << mFilename << " (" << mLineNumber << ")]";
	}

	if (mNested) {
		msg << std::endl << "\t> " << *mNested;
	}

	return msg.str();
}

const Exception* Exception::getNested() const {
	return mNested;
}

const char* Exception::className() const {
	return typeid(*this).name();
}

std::ostream& operator<<(std::ostream& os, const Exception& ex) {
	os << ex.getMessage();
	return os;
}

void Exception::message(const std::string& msg) {
	mMsg = msg;
}

const char* Exception::what() const throw() {
	return getMessage().c_str();
}

/*
 * Exception classes
 */
HOVUNI_EXCEPTION_DEFINITION(NetworkException, Exception)
HOVUNI_EXCEPTION_DEFINITION(IOException, Exception)
HOVUNI_EXCEPTION_DEFINITION(ParseException, Exception)
HOVUNI_EXCEPTION_DEFINITION(ScriptingException, Exception)
HOVUNI_EXCEPTION_DEFINITION(ConfigException, Exception)

////////////
// GLOBAL
////////////
HOVUNI_EXCEPTION_DEFINITION(UninitializedException, Exception)

////////////
// GUI
////////////
HOVUNI_EXCEPTION_DEFINITION(OverlayNotActivatedException, Exception)
HOVUNI_EXCEPTION_DEFINITION(NoParentResolutionException, Exception)

}