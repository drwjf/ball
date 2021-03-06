// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/CONCEPT/classTest.h>
#include <BALLTestConfig.h>

///////////////////////////
#include <BALL/SOLVATION/uhligCavFreeEnergyProcessor.h>
#include <BALL/FORMAT/HINFile.h>
#include <BALL/KERNEL/system.h>
#include <BALL/ENERGY/energyProcessor.h>
#include <BALL/STRUCTURE/fragmentDB.h>
///////////////////////////

START_TEST(UhligCavFreeEnergyProcessor)

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

using namespace BALL;

PRECISION(1.0)
CHECK(UhligCavFreeEnergyProcessor / Methane)
HINFile f(BALL_TEST_DATA_PATH(methane.hin));
System S;
f >> S;
f.close();
UhligCavFreeEnergyProcessor proc;
S.apply(proc);
TEST_REAL_EQUAL(proc.getEnergy(),7.08121)
TEST_EQUAL(S.countAtoms(),5)
RESULT

CHECK(UhligCavFreeEnergyProcessor::UhligCavFreeEnergyProcessor())
	UhligCavFreeEnergyProcessor* proc = new UhligCavFreeEnergyProcessor;
	TEST_NOT_EQUAL(proc, 0)
	TEST_EQUAL(proc->isValid(), true)
	TEST_REAL_EQUAL(proc->getEnergy(), 0)
	delete proc;
RESULT 

CHECK(UhligCavFreeEnergyProcessor::UhligCavFreeEnergyProcessor(const UhligCavFreeEnergyProcessor& proc))
  HINFile f(BALL_TEST_DATA_PATH(methane.hin));
  System S;
  f >> S;
  f.close();
  UhligCavFreeEnergyProcessor proc;
  S.apply(proc);
  UhligCavFreeEnergyProcessor proc2(proc); 
//  TEST_EQUAL((proc==proc2), true) 
  bool test = proc.isValid()==proc2.isValid();
  TEST_EQUAL(test, true)
  test = proc.getEnergy()==proc2.getEnergy();
  TEST_EQUAL(test, true)
RESULT
  

CHECK(UhligCavFreeEnergyProcessor::~UhligCavFreeEnergyProcessor())
	UhligCavFreeEnergyProcessor* proc = new UhligCavFreeEnergyProcessor;
	delete proc;
RESULT


CHECK(UhligCavFreeEnergyProcessor::clear())
  UhligCavFreeEnergyProcessor proc;
  proc.clear();
RESULT


CHECK(UhligCavFreeEnergyProcessor::UhligCavFreeEnergyProcessor& operator = (const UhligCavFreeEnergyProcessor& proc) + operator ==)
  HINFile f(BALL_TEST_DATA_PATH(methane.hin));
  System S;
  f >> S;
  f.close();
  UhligCavFreeEnergyProcessor proc;
  S.apply(proc);
  UhligCavFreeEnergyProcessor proc2;
//  proc2 = proc;
//  TEST_EQUAL((proc == proc2), true)
//  bool test = proc.isValid()==proc2.isValid();
//  TEST_EQUAL(test, true)
//  test = proc.getEnergy()==proc2.getEnergy();
//  TEST_EQUAL(test, true) 
RESULT


CHECK(UhligCavFreeEnergyProcessor::start())
  UhligCavFreeEnergyProcessor proc; 
  TEST_EQUAL(proc.start(), true)
RESULT

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
END_TEST
