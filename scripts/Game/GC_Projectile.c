class GC_ProjectileClass : ScriptComponentClass
{
}

class GC_Projectile : ScriptComponent
{
	[Attribute("1", UIWidgets.Auto, "Is suppression enabled for this projectile")]
	protected bool m_isSuppressionEnabled;
	
	IEntity effect;
	BaseMuzzleComponent muzzle;
	ProjectileMoveComponent move;
	
	bool IsEnabled()
	{
		return m_isSuppressionEnabled;
	}
	
	override protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		GC_SuppressionSystem suppresion = GC_SuppressionSystem.GetInstance();
		if(suppresion)
			suppresion.UnRegister(this);
	}
}