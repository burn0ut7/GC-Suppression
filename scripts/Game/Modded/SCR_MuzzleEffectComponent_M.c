modded class SCR_MuzzleEffectComponent
{
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		super.OnFired(effectEntity, muzzle, projectileEntity);

		if (ShouldInsert(muzzle))
			GC_SuppressionSystem.GetInstance().RegisterProjectile(effectEntity, muzzle, projectileEntity);
	}
	
	protected bool ShouldInsert(BaseMuzzleComponent muzzle)
	{
		// insertion criteria
		// 1. minimum distance (don't suppress yourself or squad members immediately next to you)
		// 2. maximum angle (don't suppress nearby players who aren't close to your cone of fire)
		// 3. maximum miss distance (for optimization purposes, if the shot probably passes the player at a large XZ distance, don't bother keeping track)
		
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return false;
		
		vector muzzleTransform[4];
		muzzle.GetOwner().GetWorldTransform(muzzleTransform);

	    float maxRange = GC_SuppressionSystem.GetInstance().GetMaxRange();
	
	    vector muzzlePos = muzzleTransform[3];
	    vector muzzleDir = muzzleTransform[2];
		vector playerPos = player.GetOrigin();
		
		// 1. min distance
	    float distance = vector.Distance(playerPos, muzzlePos);
	    if (distance < 10)
	        return false;
		
		vector toPlayer = playerPos - muzzlePos;
		
		// 2. maximum angle
		if (vector.DotXZ(muzzleDir, toPlayer) < 0.9)
			return false;
		
		vector muzzleDirNormalized = muzzleDir.Normalized();
		
		// 3. maximum miss dist
		
	
	    muzzlePos[1] = 0;
	    muzzleDir[1] = 0;
	    playerPos[1] = 0;
	
	    toPlayer.Normalize();
		
		//Account for wind? Only affects rockets rn
		//ChimeraWorld world = GetGame().GetWorld();
		//BaseTimeAndWeatherManagerEntity twm = world.GetTimeAndWeatherManager();
		//float windSpeed = twm.GetWindSpeed();
		//<0, 360.0>
		//float windDirction = twm.GetWindDirection();
		
	    float halfAngle = Math.Atan2(maxRange, distance);
	    float cosMax = Math.Cos(halfAngle);
	
	    float dot = vector.Dot(muzzleDir, toPlayer);
	
	    return dot >= cosMax;
	}
}
