/*=========================================================================

 Program: MAF2
 Module: mafVersionTest
 Authors: Matteo Giacomoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __CPP_UNIT_mafVersionTest_H__
#define __CPP_UNIT_mafVersionTest_H__

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

/** Test for mafMatrix; Use this suite to trace memory problems */
class mafVersionTest : public CPPUNIT_NS::TestFixture
{
public: 

	// CPPUNIT test suite
	CPPUNIT_TEST_SUITE( mafVersionTest );
	CPPUNIT_TEST(TestFixture); // just to test that the fixture has no leaks
	CPPUNIT_TEST(TestStaticAllocation);
	CPPUNIT_TEST(TestDynamicAllocation);
	CPPUNIT_TEST(TestGetMAFVersion);
	CPPUNIT_TEST(TestGetMAFMajorVersion);
	CPPUNIT_TEST(TestGetMAFMinorVersion);
	CPPUNIT_TEST(TestGetMAFBuildVersion);
	CPPUNIT_TEST(TestGetMAFSourceVersion);
	CPPUNIT_TEST_SUITE_END();

private:
	void TestFixture();
	void TestStaticAllocation();
	void TestDynamicAllocation();
	void TestGetMAFVersion();
	void TestGetMAFMajorVersion();
	void TestGetMAFMinorVersion();
	void TestGetMAFBuildVersion();
	void TestGetMAFSourceVersion();
};

int
main( int argc, char* argv[] )
{
	// Create the event manager and test controller
	CPPUNIT_NS::TestResult controller;

	// Add a listener that collects test result
	CPPUNIT_NS::TestResultCollector result;
	controller.addListener( &result );        

	// Add a listener that print dots as test run.
	CPPUNIT_NS::BriefTestProgressListener progress;
	controller.addListener( &progress );      

	// Add the top suite to the test runner
	CPPUNIT_NS::TestRunner runner;
	runner.addTest( mafVersionTest::suite());
	runner.run( controller );

	// Print test in a compiler compatible format.
	CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
	outputter.write(); 

	return result.wasSuccessful() ? 0 : 1;
}
#endif