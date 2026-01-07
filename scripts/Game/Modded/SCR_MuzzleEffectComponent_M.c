modded class SCR_MuzzleEffectComponent
{
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		super.OnFired(effectEntity, muzzle, projectileEntity);

		if(IsPlayerInConeXZ(muzzle))
			GC_SuppressionSystem.Register(effectEntity, muzzle, projectileEntity);
	}
	
	bool IsPlayerInConeXZ(BaseMuzzleComponent muzzle)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(!player)
			return false;
		
		vector muzzleTransform[4];
		muzzle.GetOwner().GetWorldTransform(muzzleTransform);

	    float maxRange = GC_SuppressionSystem.GetInstance().GetMaxRange();
	
	    vector muzzlePos = muzzleTransform[3];
	    vector muzzleDir = muzzleTransform[2];
		vector playerPos = player.GetOrigin();
	
	    muzzlePos[1] = 0;
	    muzzleDir[1] = 0;
	    playerPos[1] = 0;
	
	    vector toPlayer = playerPos - muzzlePos;
	
	    float distance = toPlayer.Length();
		
	    if (distance < 0.1)
	        return true;
	
	    toPlayer.Normalize();
	
	    // Dynamic half-angle (radians)
	    float halfAngle = Math.Atan2(maxRange, distance);
	    float cosMax = Math.Cos(halfAngle);
	
	    float dot = vector.Dot(muzzleDir, toPlayer);
	
	    return dot >= cosMax;
	}
}
