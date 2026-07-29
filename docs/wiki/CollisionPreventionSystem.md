---
title: CollisionPreventionSystem
description: Collision prevention system.
---

# Introduction

Het collision prevention system gebruikt havok om mogelijke botsingen te voorspellen.
Er is één interface namelijk EntityCollision. Voor deze interface zijn twee implementaties
beide laten toe om objecten te detecteren de eerste eenvoudige variant is minder resource intensief maar ook wel minder krachtig, de tweede is nog altijd heel cheap kwa rekentijd maar kan wel veel juister detecties doen.

# Details

## Interface

```
class EntityCollision {

public:

	EntityCollision(){
	}

	virtual ~EntityCollision(){
	}

	/**
	 * Called when an object overlap detected with the phantom shape
	 *
	 * @param ??
	 */
	virtual void onOverlapEnter( ) = 0;

	/**
	 * Called when an object overlap is removed with the phantom shape
	 *
	 * @param ??
	 */
	virtual void onOverlapLeave( ) = 0;	
};
```

## Simple

http://delahaye.it/uploader/files/collision_prevention_simple.jpg

## Advanced

http://delahaye.it/uploader/files/collision_prevention_advanced.jpg
