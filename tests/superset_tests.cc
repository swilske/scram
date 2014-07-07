#include <gtest/gtest.h>

#include "error.h"
#include "risk_analysis.h"
#include "superset.h"

using namespace scram;

// Test AddPrime function
TEST(SupersetTest, AddPrime) {
  Superset* sset = new Superset();

  std::string prime_event = "valveone";  // Prime event in the tree.
  std::string new_prime_event = "valvetwo";  // Prime event not in the tree.

  std::set<std::string> primes;
  // Expect empty superset at beginning.
  EXPECT_EQ(sset->primes(), primes);
  // Add members.
  EXPECT_NO_THROW(sset->AddPrimary(prime_event));
  // Test if members were added successfully.
  primes.insert(prime_event);
  EXPECT_EQ(sset->primes(), primes);
  // Test if repeated addition handled in a set.
  EXPECT_NO_THROW(sset->AddPrimary(prime_event));
  EXPECT_EQ(sset->primes(), primes);

  // Add the second element.
  EXPECT_NO_THROW(sset->AddPrimary(new_prime_event));
  // Test if members were added successfully.
  primes.insert(new_prime_event);
  EXPECT_EQ(sset->primes(), primes);
  // Test if repeated addition handled in a set.
  EXPECT_NO_THROW(sset->AddPrimary(new_prime_event));
  EXPECT_EQ(sset->primes(), primes);

  delete sset;
}

// Test AddInter function.
TEST(SupersetTest, AddInter) {
  Superset* sset = new Superset();

  std::string inter_event = "trainone";  // Inter event in the tree.
  std::string new_inter_event = "traintwo";  // Inter event not in the tree.

  std::set<std::string> inters;
  // Expect empty superset at beginning.
  EXPECT_EQ(sset->inters(), inters);
  // Add members.
  EXPECT_NO_THROW(sset->AddInter(inter_event));
  // Test if members were added successfully.
  inters.insert(inter_event);
  EXPECT_EQ(sset->inters(), inters);
  // Test if repeated addition handled in a set.
  EXPECT_NO_THROW(sset->AddInter(new_inter_event));
  inters.insert(new_inter_event);
  EXPECT_EQ(sset->inters(), inters);

  delete sset;
}

// Test Insert function
TEST(SupersetTest, Insert) {
  Superset* sset_one = new Superset();
  Superset* sset_two = new Superset();

  std::string prime_event_one = "valveone";
  std::string inter_event_one = "trainone";
  std::string prime_event_two = "valvetwo";
  std::string inter_event_two = "traintwo";

  sset_one->AddPrimary(prime_event_one);
  sset_two->AddPrimary(prime_event_two);
  sset_one->AddInter(inter_event_one);
  sset_two->AddInter(inter_event_two);

  sset_one->Insert(sset_two);

  std::set<std::string> primes;
  std::set<std::string> inters;
  primes.insert(prime_event_one);
  primes.insert(prime_event_two);
  inters.insert(inter_event_one);
  inters.insert(inter_event_two);

  EXPECT_EQ(sset_one->primes(), primes);
  EXPECT_EQ(sset_one->inters(), inters);

  delete sset_one;
  delete sset_two;
}

// Test PopInter function
TEST(SupersetTest, PopInter) {
  Superset* sset = new Superset();
  // Fail to pop when the set is empty.
  EXPECT_THROW(sset->PopInter(), ValueError);
  // Add intermediate event into the set.
  std::string inter_event = "trainone";
  sset->AddInter(inter_event);
  EXPECT_EQ(sset->PopInter(), inter_event);
  // Test emptyness after poping the only inserted event.
  EXPECT_THROW(sset->PopInter(), ValueError);

  delete sset;
}

// Test NumOfPrimeEvents function
TEST(SupersetTest, NumOfPrimeEvents) {
  Superset* sset = new Superset();
  EXPECT_EQ(sset->NumOfPrimeEvents(), 0);  // Empty case.
  sset->AddPrimary("pumpone");
  EXPECT_EQ(sset->NumOfPrimeEvents(), 1);
  // Repeat addition to check if the size changes. It should not.
  sset->AddPrimary("pumpone");
  EXPECT_EQ(sset->NumOfPrimeEvents(), 1);
  // Add a new member.
  sset->AddPrimary("pumptwo");
  EXPECT_EQ(sset->NumOfPrimeEvents(), 2);

  delete sset;
}

// Test NumOfInterEvents function
TEST(SupersetTest, NumOfInterEvents) {
  Superset* sset = new Superset();
  EXPECT_EQ(sset->NumOfInterEvents(), 0);  // Empty case.
  sset->AddInter("trainone");
  EXPECT_EQ(sset->NumOfInterEvents(), 1);
  // Repeat addition to check if the size changes. It should not.
  sset->AddInter("trainone");
  EXPECT_EQ(sset->NumOfInterEvents(), 1);
  // Add and delete a new member.
  sset->AddInter("traintwo");
  EXPECT_EQ(sset->NumOfInterEvents(), 2);
  sset->PopInter();
  EXPECT_EQ(sset->NumOfInterEvents(), 1);
  // Empty the set.
  sset->PopInter();
  EXPECT_EQ(sset->NumOfInterEvents(), 0);

  delete sset;
}

// Test corner case with deleting the superset instance.
TEST(SupersetTest, DeleteSet) {
  Superset* sset = new Superset();
  std::string prime_event = "valveone";
  std::string inter_event = "trainone";
  sset->AddPrimary(prime_event);
  sset->AddInter(inter_event);
  std::set<std::string> primes;
  std::set<std::string> inters;
  primes = sset->primes();
  inters = sset->inters();
  primes.insert("invalid");
  EXPECT_EQ(sset->primes().size(), 1);
  delete sset;  // This is a wild test that should be deleted in future.
  EXPECT_EQ(primes.size(), 2);
  EXPECT_EQ(inters.size(), 1);
}
