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
	
	    vector muzzlePos = muzzleTransform[3];
	    vector muzzleDir = muzzleTransform[2].Normalized();
		vector playerPos = player.GetOrigin();
		
		// 1. min distance
	    float distance = vector.Distance(playerPos, muzzlePos);
	    if (distance < GC_SuppressionSystem.GetInstance().GetMinRange())
	        return false;
		
		//This mostly works. Upclose projectiles will not be tracked due to being outside angle
		// 2. maximum angle (~25° forward cone)
		vector toPlayer = playerPos - muzzlePos;
		if (vector.DotXZ(muzzleDir, toPlayer.Normalized()) < 0.975)
			return false;
		
		// 3. maximum miss distance
		float t = vector.DotXZ(toPlayer, muzzleDir);
		if (t < 0) // technically double checking. leaving still in case above was changed.
			return false;
		vector passPoint = muzzlePos + muzzleDir * t;
		if (vector.DistanceXZ(playerPos, passPoint) > 100) // 100 is kind of arbitrary but should be enough
			return false;
		
	    return true;
	}
}
