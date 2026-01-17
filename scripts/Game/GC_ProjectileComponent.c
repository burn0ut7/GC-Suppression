class GC_ProjectileComponentClass : ScriptComponentClass
{
}

class GC_ProjectileComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.Auto, "Is suppression enabled for this projectile")]
	protected bool m_isSuppressionEnabled;
	
	IEntity entity;
	ProjectileMoveComponent move;
	vector position;
	
	bool IsEnabled()
	{
		return m_isSuppressionEnabled;
	}
	
	override protected void OnDelete(IEntity owner)
	{
		GC_SuppressionSystem suppresion = GC_SuppressionSystem.GetInstance();
		if(suppresion)
			suppresion.UnregisterProjectile(this);
		
		super.OnDelete(owner);
	}
}