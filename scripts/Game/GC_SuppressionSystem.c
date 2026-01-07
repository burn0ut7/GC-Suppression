class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression")]
	protected float m_maxRange;
	
	static protected GC_SuppressionSystem m_instance;
	
	protected ref array<GC_Projectile> m_projectiles = {};
	
	protected float m_Suppression = 0;
	
	protected float m_maxRangeSq;
	

	override protected void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);
		
		UpdateProjectiles();
	}
	
	protected void UpdateProjectiles()
	{
		//Print("GC | UpdateProjectiles: " + m_projectiles.Count());
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(!player)
			return;
		
		vector playerPos = GetGame().GetPlayerController().GetControlledEntity().GetOrigin();
		
		for (int i = m_projectiles.Count() - 1; i >= 0; i--)
		{
		    GC_Projectile projectile = m_projectiles[i];
		
		    vector projPos = projectile.GetOwner().GetOrigin();

		    // Direction from projectile to player
		    vector toPlayer = playerPos - projPos;
		
		    // Projectile velocity
		    vector velocity = projectile.move.GetVelocity();
		
		    // Check if projectile is moving toward the player
		    float approach = vector.Dot(toPlayer, velocity);
		
		    if (approach <= 0)
		    {
		        // Projectile is no longer approaching → check distance
		        float distSq = vector.DistanceSq(projPos, playerPos);
		
		        if (distSq <= m_maxRangeSq)
		        {
		            ApplySuppression(projectile);
		        }
		
		        // Remove projectile regardless
				Print("GC | Projectile Remove: " + projectile);
		        m_projectiles.Remove(i);
		        continue;
		    }
		
		    // Projectile is still approaching → do nothing, keep tracking
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
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(!player)
			return;
		
		GC_Projectile projComp = GC_Projectile.Cast(projectile.FindComponent(GC_Projectile));
		if(!projComp)
			return;
		
		if(!projComp.IsEnabled())
			return;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if(!moveComp)
			return;
		
		projComp.effect = effect;
		projComp.muzzle = muzzle;
		projComp.move = moveComp;

		m_instance.m_projectiles.Insert(projComp);
		
		Print("GC | Projectile Registered: " + projectile);
	}
	
	static void UnRegister(GC_Projectile projectile)
	{
		//Try apply hit location
		
		Print("GC | Projectile UnRegistered: " + projectile);
		
		m_instance.m_projectiles.RemoveItem(projectile);
	}
	
	float GetMaxRange()
	{
		return m_maxRange;
	}
	
	override void OnInit()
	{
		super.OnInit();
	
		m_maxRangeSq = Math.Pow(m_maxRange, 2);
		m_instance = this;
	}
	
	static GC_SuppressionSystem GetInstance()
	{
		return m_instance;
	}
}
