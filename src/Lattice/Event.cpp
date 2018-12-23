#include "Windows.h"
#include "Misc.h"
#include "Event.h"

EventID Event::GetID() const
{
	return raw & MAXUINT8;
}
EventID Event::SetID(EventID n)
{
	raw = (raw & ~MAXUINT8) | n;
	return n;
}

int Event::GetDifficulty() const
{
	return (raw >> 8) & 3;
}
int Event::SetDifficulty(int f)
{
	raw = (raw & ~(3 << 8)) | ((f & 3) << 8);
	return f;
}

bool Event::GetIlluminated() const
{
	return !!(raw & (1 << 10));
}
bool Event::SetIlluminated(bool f)
{
	if (f)
		raw |= (1 << 10);
	else
		raw &= ~(1 << 10);
	return f;
}

bool Event::GetActive() const
{
	return !!(raw & (1 << 11));
}
bool Event::SetActive(bool f)
{
	if (f)
		raw |= (1 << 11);
	else
		raw &= ~(1 << 11);
	return f;
}

int Event::GetParameter(int offset, int size) const
{
	const int realOffset = offset + 12;
	if (size >= 0) //unsigned
		return ((1 << size) - 1) & (raw >> realOffset);
	//signed
	const int realSize = 1 << -size;
	int result = (realSize - 1) & (raw >> realOffset);
	if ((result & (realSize >> 1)) == realSize >> 1)
		result -= realSize;
	return result;
}

int Event::SetParameter(int offset, int size, int value)
{
	//if (size == 0 || offset + size > 20) return value;
	int nVal = value; //new int, so the return value is unchanged
	size = abs(size);
	raw &= ~(((1 << size) - 1) << (offset + 12)); //erase (size) bits at (offset)
	if (value < 0) nVal += 1 << size;
	nVal &= (1 << size) - 1; //truncate nVal to its first (size) bits
	raw |= (nVal << (offset + 12)); //add the new (size) bits at (offset)
	return value;
}

sf::Uint32 Event::operator=(sf::Uint32 other)
{
	return raw = other;
}
