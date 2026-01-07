class GC_SupressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression")]
	protected float m_maxRange;
	
	protected static GC_SupressionSystem m_instance;
	
	protected ref array<GC_Projectile> m_projectiles = {};
	
	protected float m_Suppression = 0;
	
	protected static float m_maxRangeSq;
	
	protected void UpdateProjectiles()
	{
		//Print("GC | UpdateProjectiles: " + m_projectiles.Count());
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(!player)
			return;
		
		vector playerPos = player.GetOrigin();

		for (int i = m_projectiles.Count() - 1; i >= 0; i--)
		{
			GC_Projectile projectile = m_projectiles[i];
			
			vector projPos = projectile.GetOwner().GetOrigin();
			vector toPlayer = playerPos - projPos;
			
			vector velocity = projectile.move.GetVelocity();
			
			float approach = vector.Dot(toPlayer, velocity);
			
			if (approach <= 0)
			{
				float distSq = vector.DistanceSq(projPos, playerPos);
				
				if (distSq <= m_maxRangeSq)
				ApplySuppression(projectile);
				
				Print("GC | Projectile Remove: " + projectile);
				m_projectiles.Remove(i);
			}
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
	
	static void OnProjectileDelete(GC_Projectile projectile)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(player)
		{
			vector projPos = projectile.GetOwner().GetOrigin();
			float distSq = vector.DistanceSq(projPos, player.GetOrigin());
		
			if(distSq <= m_maxRangeSq)
			
		}
		
		m_instance.m_projectiles.RemoveItem(projectile);
	}

	float GetMaxRange()
	{
		return m_maxRange;
	}
	
	static GC_SupressionSystem GetInstance()
	{
		return m_instance;
	}
	
	override protected void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);
		
		UpdateProjectiles();
	}
	
	override void OnInit()
	{
		super.OnInit();
	
		m_maxRangeSq = Math.Pow(m_maxRange, 2);
		m_instance = this;
	}
	
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo
			.SetAbstract(false)
			.SetUnique(true)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.Frame);
	}
}
