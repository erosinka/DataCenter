#DEFINE BOOST_TEST_MODULE basic

#include "test/unit/basic/helper.h"
#include "src/server_farm.h"

BOOST_AUTO_TEST_CASE( test_spike_t )
{
NRNMPI_Spike spike;
BOOST_CHECK_EQUAL(spike.gid, 0);
BOOST_CHECK_CLOSE( spike.spiketime, 0.0, 0.0001 );
int gid = 1;
double time = 1.2;
NRNMPI_Spike spike2 (gid, time);
BOOST_CHECK_EQUAL(spike2.gid, 1);
BOOST_CHECK_CLOSE(spike2.spiketime, 1.2, 0.0001);
}
