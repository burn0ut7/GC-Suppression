#ifdef PS_LOBBY
modded class GC_SuppressionSystem
{
	override void RegisterProjectile(IEntity projectile)
	{
		PS_PlayableControllerComponent playable = PS_PlayableControllerComponent.GetInstance();
		if(playable.IsSpectating())
			return;
		
		super.RegisterProjectile(projectile);
	}
	
	override protected void AddSuppression(float suppression)
	{
		PS_PlayableControllerComponent playable = PS_PlayableControllerComponent.GetInstance();
		if(playable.IsSpectating())
			return;
		
		super.AddSuppression(suppression);
	}
}
#endif