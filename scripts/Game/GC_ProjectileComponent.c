class GC_ProjectileComponentClass : ScriptComponentClass
{
}

class GC_ProjectileComponent : ScriptComponent
{
	vector position;

	override protected void OnDelete(IEntity owner)
	{
		GC_SuppressionSystem suppresion = GC_SuppressionSystem.GetInstance();
		if(suppresion)
			suppresion.UnregisterProjectile(this);
		
		super.OnDelete(owner);
	}
}