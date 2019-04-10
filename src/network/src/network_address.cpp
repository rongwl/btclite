#include "network_address.h"

namespace btclite {

bool NetAddr::IsRFC1918() const
{
	return true;
}

bool NetAddr::IsRFC2544() const
{
	return true;
}

bool NetAddr::IsRFC6598() const
{
	return true;
}

bool NetAddr::IsRFC5737() const
{
	return true;
}

bool NetAddr::IsRFC3849() const
{
	return true;
}

bool NetAddr::IsRFC3927() const
{
	return true;
}

bool NetAddr::IsRFC3964() const
{
	return true;
}

bool NetAddr::IsRFC4193() const
{
	return true;
}

bool NetAddr::IsRFC4380() const
{
	return true;
}

bool NetAddr::IsRFC4843() const
{
	return true;
}

bool NetAddr::IsRFC4862() const
{
	return true;
}

bool NetAddr::IsRFC6052() const
{
	return true;
}

bool NetAddr::IsRFC6145() const
{
	return true;
}

bool NetAddr::IsLocal() const
{
	return true;
}

bool NetAddr::IsRoutable() const
{
	return true;
}

bool NetAddr::IsInternal() const
{
	return true;
}

bool NetAddr::IsValid() const
{
	return true;
}

void NetAddr::Clear()
{
	addr_.clear_timestamp();
	addr_.clear_services();
	addr_.clear_ip();
	addr_.clear_port();
}

} // namespace btclite
