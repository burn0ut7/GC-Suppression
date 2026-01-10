modded class SCR_MuzzleEffectComponent
{
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		super.OnFired(effectEntity, muzzle, projectileEntity);

		if (ShouldInsert(projectileEntity))
			GC_SuppressionSystem.GetInstance().RegisterProjectile(projectileEntity);
	}
	
	protected bool ShouldInsert(IEntity projectile)
	{
		// insertion criteria
		// 1. minimum distance (don't suppress yourself or squad members immediately next to you)
		// 2. maximum angle (don't suppress nearby players who aren't close to your cone of fire)
		// 3. maximum miss distance (for optimization purposes, if the shot probably passes the player at a large XZ distance, don't bother keeping track)
	
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return false;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if (!moveComp)
			return false;
		
		vector velocity = moveComp.GetVelocity();
		if (velocity.LengthSq() <= 0.0001)
			return false;
		
		velocity.Normalize();
		
		vector projPos = projectile.GetOrigin();
		
		SCR_ChimeraCharacter cc = SCR_ChimeraCharacter.Cast(player);
		vector playerEyePos = cc.EyePosition();

		// 1. min distance
	    float distance = vector.Distance(playerEyePos, projPos);
	    if (distance < GC_SuppressionSystem.GetInstance().GetMinRange())
	        return false;
	
		//This mostly works. Upclose projectiles will not be tracked due to being outside angle
		// 2. maximum angle (~25° forward cone)
		vector toPlayer = playerEyePos - projPos;
		if (vector.DotXZ(velocity, toPlayer.Normalized()) < 0.975)
			return false;
		
		// 3. maximum miss distance
		float t = vector.DotXZ(toPlayer, velocity);
		if (t < 0) // technically double checking. leaving still in case above was changed.
			return false;
		vector passPoint = projPos + velocity * t;
		if (vector.DistanceXZ(playerEyePos, passPoint) > 100) // 100 is kind of arbitrary but should be enough
			return false;
		
	    return true;
	}
}
