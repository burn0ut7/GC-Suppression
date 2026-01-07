modded class SCR_MuzzleEffectComponent
{
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		super.OnFired(effectEntity, muzzle, projectileEntity);
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(!player)
			return;

		vector transform[4];
		muzzle.GetOwner().GetWorldTransform(transform);

		vector toPlayer = player.GetOrigin() - transform[3];
		toPlayer[1] = 0;
		toPlayer.Normalize();
		
		float dot = vector.Dot(transform[2], toPlayer);
		
		if(dot >= 0)
			GC_SupressionController.Register(effectEntity, muzzle, projectileEntity);
	}
}
