/*
 * Native class adapter template for 'rpcTests.StringNativeClass'.
 * WARNING: remember to copy this file to your project dir before you begin to change it.
 * Generated by the Coral Compiler v0.7.0 on Thu Mar 01 15:35:26 2012.
 */

#include "StringNativeClass_Adapter.h"

namespace rpcTests {

const std::string& StringNativeClass_Adapter::getValue( rpcTests::StringNativeClass& instance )
{
	return instance.data;
}

void StringNativeClass_Adapter::setValue( rpcTests::StringNativeClass& instance, const std::string& value )
{
	instance.data = value;
}

} // namespace rpcTests
