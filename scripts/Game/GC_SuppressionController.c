class GC_SupressionControllerClass : ScriptComponentClass
{
}

class GC_SupressionController : ScriptComponent
{
	[Attribute("10", UIWidgets.Auto, "Max Distance in meters for a bullet to apply suppression")]
	protected float m_maxDistance;
	
	static protected GC_SupressionController m_instance;
	
	protected ref array<GC_Projectile> m_projectiles = {};
	
	protected float m_Suppression = 0;
	
	//Cache mass for each bullet type
	
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		UpdateProjectiles();
	}
	
	protected void UpdateProjectiles()
	{
		Print("GC | UpdateProjectiles: " + m_projectiles.Count());
		
		foreach(GC_Projectile projectile : m_projectiles)
		{
			vector currentPos = projectile.GetOwner().GetOrigin();
			
			float distance = vector.DistanceSq(projectile.position, currentPos);
			
			if(distance <= m_maxDistance)
				ApplySuppression(projectile);
			
			projectile.position = currentPos;
		}
	}
	
	void ApplySuppression(GC_Projectile projectile)
	{
		Print("GC | ApplySuppression: " + projectile);
		
		//ShellMoveComponent shellComp = ShellMoveComponent.Cast(projectile.FindComponent(ShellMoveComponent));
		//proj.shell = shellComp.GetComponentSource(projectile);
	}
	
	static void Register(IEntity effect, BaseMuzzleComponent muzzle, IEntity projectile)
	{
		if(!m_instance)
			return;
		
		GC_Projectile projComp = GC_Projectile.Cast(projectile.FindComponent(GC_Projectile));
		if(!projComp)
			return;
		
		if(!projComp.IsEnabled())
			return;
		
		projComp.effect = effect;
		projComp.muzzle = muzzle;

		m_instance.m_projectiles.Insert(projComp);
		
		Print("GC | Projectile Registered: " + projectile);
	}
	
	static void UnRegister(GC_Projectile projectile)
	{
		//Try apply hit location
		
		m_instance.m_projectiles.RemoveItem(projectile);
	}
		
	override protected void OnPostInit(IEntity owner)
	{
		m_maxDistance *= m_maxDistance;
		m_instance = this;
		
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	static GC_SupressionController GetInstance()
	{
		return m_instance;
	}
}
