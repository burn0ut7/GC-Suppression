class GC_ProjectileClass : ScriptComponentClass
{
}

class GC_Projectile : ScriptComponent
{
	[Attribute("1", UIWidgets.Auto, "Is suppression enabled for this projectile")]
	protected bool m_isSuppressionEnabled;
	
	IEntity effect;
	BaseMuzzleComponent muzzle;
	vector position;
	
	bool IsEnabled()
	{
		return m_isSuppressionEnabled;
	}
	
	override protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		GC_SupressionController suppresion = GC_SupressionController.GetInstance();
		if(suppresion)
			suppresion.UnRegister(this);
	}
}