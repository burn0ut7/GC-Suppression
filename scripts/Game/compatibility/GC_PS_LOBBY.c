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
	
	override void HandleExplosion(IEntity source, vector transform[3], IEntity explosion, float multiplier)
	{
		PS_PlayableControllerComponent playable = PS_PlayableControllerComponent.GetInstance();
		if(playable.IsSpectating())
			return;
		
		super.HandleExplosion(source, transform, explosion, multiplier);
	}
	
	override void HandleBulletImpact(IEntity bullet, vector transform[3], float multiplier, float distance, float speed)
	{
		PS_PlayableControllerComponent playable = PS_PlayableControllerComponent.GetInstance();
		if(playable.IsSpectating())
			return;
		
		super.HandleBulletImpact(bullet, transform, multiplier, distance, speed);
	}
}

modded class PS_PlayableControllerComponent : ScriptComponent
{
	override bool IsSpectating()
	{
		PrintFormat("GC Supp | IsSpectating:%1", m_isSpectating);
		
		return m_isSpectating;
	}

}

#endif