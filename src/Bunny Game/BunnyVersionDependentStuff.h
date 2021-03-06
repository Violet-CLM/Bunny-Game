#pragma once
#include "BunnySamples.h"

extern bool VersionTSF; //this has to be checked in too many random places for it not to be global, unfortunately

//character animations, using their TSF values--should be used as indices
namespace RabbitAnims {
	enum {
		AIRBOARD,
		AIRBOARDTURN,
		BUTTSTOMPLAND,
		CORPSE,
		DIE,
		DIVE,
		DIVEFIREQUIT,
		DIVEFIRERIGHT,
		DIVEUP,
		EARBRACHIATE,
		ENDOFLEVEL,
		FALL,
		FALLBUTTSTOMP,
		FALLLAND,
		FIRE,
		FIREUP,
		FIREUPQUIT,
		FROG,
		HANGFIREQUIT,
		HANGFIREREST,
		HANGFIREUP,
		HANGIDLE1,
		HANGIDLE2,
		HANGINGFIREQUIT,
		HANGINGFIRERIGHT,
		HELICOPTER,
		HELICOPTERFIREQUIT,
		HELICOPTERFIRERIGHT,
		HPOLE,
		HURT,
		IDLE1,
		IDLE2,
		IDLE3,
		IDLE4,
		IDLE5,
		JUMPFIREQUIT,
		JUMPFIRERIGHT,
		JUMPING1,
		JUMPING2,
		JUMPING3,
		LEDGEWIGGLE,
		LIFT,
		LIFTJUMP,
		LIFTLAND,
		LOOKUP,
		LOOPY,
		PUSH,
		QUIT,
		REV1,
		REV2,
		REV3,
		RIGHTFALL,
		RIGHTJUMP,
		ROLLING,
		RUN1,
		RUN2,
		RUN3,
		SKID1,
		SKID2,
		SKID3,
		SPRING,
		STAND,
		STATIONARYJUMP,
		STATIONARYJUMPEND,
		STATIONARYJUMPSTART,
		STONED,
		SWIMDOWN,
		SWIMRIGHT,
		SWIMTURN1,
		SWIMTURN2,
		SWIMUP,
		SWINGINGVINE,
		TELEPORT,
		TELEPORTFALL,
		TELEPORTFALLING,
		TELEPORTFALLTELEPORT,
		TELEPORTSTAND,
		VPOLE,

		LAST
	};
}
extern int RabbitAnimIDs[RabbitAnims::LAST];

typedef int GetAnimationIDFunc(int);

extern GetAnimationIDFunc* GetVersionSpecificAnimationID;
#define ConvertIDsToEnum(SetID, SampleID) Samples::s ## SetID ## _ ## SampleID
#define PlaySamplePriority(SetID, SampleID) HostLevel.StartSound(GetVersionSpecificAnimationID(AnimSets::SetID), ConvertIDsToEnum(SetID, SampleID))
#define PlaySample(SetID, SampleID, ...) HostLevel.StartSound(GetVersionSpecificAnimationID(AnimSets::SetID), ConvertIDsToEnum(SetID, SampleID), __VA_ARGS__)
#define PlaySampleAtObject(SetID, SampleID) PlaySample(SetID, SampleID, sf::Vector2f(PositionX, PositionY))

void InitializeRabbitAnimIDs();
bool IsTSF(bool & isTSF);
