class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression")]
	protected float m_fMaxRange;
	
	[Attribute("50", UIWidgets.Auto, "Distance up to which cover is recognized (meters)")]
	protected float m_fCoverTraceLength;
	
	[Attribute("5.0", UIWidgets.Auto, "Seconds of no new suppression before recovery starts")]
	protected float m_fRecoveryDelay;
	
	// Suppression recovered per second once recovery starts
	[Attribute("0.1", UIWidgets.Auto, "Suppression recovery speed per second")]
	protected float m_fRecoveryRate;
	
	[Attribute("1", UIWidgets.Auto, "Multiplier for the visual intensity of the suppression effect")]
	protected float m_fEffectIntensity;
	
	// How much mass contributes to suppression
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to projectile mass for suppression")]
	protected float m_fMassMultiplier;
	
	// How much speed contributes to suppression
	[Attribute("0.001", UIWidgets.Auto, "Multiplier applied to projectile speed for suppression")]
	protected float m_fSpeedMultiplier;
	
	// Global scale for suppression added per bullet
	[Attribute("0.05", UIWidgets.Auto, "Global suppression multiplier per bullet")]
	protected float m_fBaseSuppressionMultiplier;
	
	// Suppression multiplier when in cover (0.5 = -50%)
	[Attribute("0.5", UIWidgets.Auto, "Suppression multiplier when target is in cover (0.5 = -50%)")]
	protected float m_fCoverMultiplier;

	//! Projectiles tracked by the system
	protected ref array<GC_ProjectileComponent> m_aProjectiles = {};
	
	//! Player suppression value 0-1
	protected float m_fSuppression = 0;
	
	protected int m_iLastUpdateMs = 0;
	protected int m_iLastSuppressionMs = 0;
	
	static GC_SuppressionSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;
		return GC_SuppressionSystem.Cast(world.FindSystem(GC_SuppressionSystem));
	}

	override void OnInit()
	{
		super.OnInit();
	}

	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo
			.SetAbstract(false)
			.SetUnique(true)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.AfterPhysics);
	}

	override protected void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);
		
		if (m_aProjectiles.IsEmpty() && m_fSuppression == 0)
			return;
		
		UpdateProjectiles();
		UpdateSuppression();
	}

	protected void UpdateProjectiles()
	{
		//Print("GC | UpdateProjectiles: " + m_aProjectiles.Count());
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		
		SCR_ChimeraCharacter cc = SCR_ChimeraCharacter.Cast(player);
		vector playerEyePos = cc.EyePosition();
		
		for (int i = m_aProjectiles.Count() - 1; i >= 0; i--)
		{
		
			GC_ProjectileComponent projectile = m_aProjectiles[i];
			
		    vector projPos = projectile.GetOwner().GetOrigin();
		
		    // Check if projectile is moving toward the player
		    float approach = vector.Dot(playerEyePos - projPos, projectile.move.GetVelocity());
			
		    if (approach <= 0)
		    {
		        // Projectile is no longer approaching → check distance
		        float dist = vector.Distance(projPos, playerEyePos);

				if (dist <= m_fMaxRange)
				{
					// Cover check should move to seperate method so it can be reused for hits around a player
					// "perfect shot" projectile trace towards player head to check whether it would have been in some kind of cover
					TraceParam tp = MakeTraceParam(playerEyePos - projectile.move.GetVelocity().Normalized() * m_fCoverTraceLength, playerEyePos, TraceFlags.ENTS | TraceFlags.WORLD);
					tp.Exclude = player;
					float trace = GetWorld().TraceMove(tp);
					CreateDebugCircle(projPos, Color.GREEN);
					AddSuppression(projectile, dist, m_fCoverTraceLength * trace > m_fCoverTraceLength - 0.1);
				}
				else
				{
					CreateDebugCircle(projPos, Color.PINK);
				}
				
		        // Remove projectile regardless
				//Print("GC | Projectile Remove: " + projectile);
		        m_aProjectiles.Remove(i);
		        continue;
		    }
			else
				CreateDebugCircle(projPos);
		
		    // Projectile is still approaching → do nothing, keep tracking
		}
	}
	
	void AddSuppression(GC_ProjectileComponent projectile, float distance, bool inCover)
	{
		if (!projectile)
			return;
	
		float mass;
		BaseContainer container = projectile.move.GetComponentSource(projectile.GetOwner());
		if (container)
			container.Get("Mass", mass);
	
		float speed = projectile.move.GetVelocity().Length();
	
		float distanceTerm = 0.0;
		if (distance <= 0.0)
			distanceTerm = 1.0;
		else if (distance < m_fMaxRange)
			distanceTerm = 1.0 - (distance / m_fMaxRange);

		float addSuppression = 0.0;
		addSuppression += (mass * m_fMassMultiplier);
		addSuppression += (speed * m_fSpeedMultiplier);
		addSuppression *= distanceTerm;
	
		// Apply cover multiplier
		if (inCover)
			addSuppression = addSuppression * m_fCoverMultiplier;
	
		// Global scale
		addSuppression = addSuppression * m_fBaseSuppressionMultiplier;
		
		m_fSuppression = Math.Clamp(m_fSuppression + addSuppression, 0, 1);
		
		m_iLastSuppressionMs = System.GetTickCount();
		
		UpdateEffect();
		
		PrintFormat("GC | AddSuppression mass=%1 speed=%2 dist=%3 cover=%4 add=%5 total=%6",
			mass, speed, distance, inCover, addSuppression, m_fSuppression);
	}
	
	protected void UpdateSuppression()
	{
		int now = System.GetTickCount();
	
		float deltaSec = (now - m_iLastUpdateMs) / 1000.0;
		m_iLastUpdateMs = now;
	
		if (m_fSuppression <= 0)
			return;
	
		float sinceSuppSec = (now - m_iLastSuppressionMs) / 1000.0;
	
		if (sinceSuppSec < m_fRecoveryDelay)
			return;
	
		float decay = m_fRecoveryRate * deltaSec;

		m_fSuppression = Math.Clamp(m_fSuppression - decay, 0, 1);
		
		UpdateEffect();
		
		PrintFormat("GC | Recover: " + decay);
	}
	
	protected void UpdateEffect()
	{
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if(sse)
			sse.UpdateSuppresion();
	}

	
	void RegisterProjectile(IEntity effect, BaseMuzzleComponent muzzle, IEntity projectile)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		
		GC_ProjectileComponent projComp = GC_ProjectileComponent.Cast(projectile.FindComponent(GC_ProjectileComponent));
		if (!projComp)
			return;
		
		if (!projComp.IsEnabled())
			return;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if (!moveComp)
			return;
		
		projComp.effect = effect;
		projComp.muzzle = muzzle;
		projComp.move = moveComp;

		m_aProjectiles.Insert(projComp);
		
		Print("GC | Projectile Registered: " + projectile);
	}
	
	void UnregisterProjectile(GC_ProjectileComponent projectile)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(player)
		{
			vector projPos = projectile.GetOwner().GetOrigin();
			float dist = vector.Distance(projPos, player.GetOrigin());
		
			//if (dist <= m_fMaxRange)
			//Get if entity hit nearby, add extra supression
			//Should prob be seperate method
		}
		
		m_aProjectiles.RemoveItem(projectile);
	}

	float GetAmount()
	{
		return m_fSuppression;
	}
	
	float GetMaxRange()
	{
		return m_fMaxRange;
	}
	
	//! Debug shapes, remove later
	protected ref array<ref Shape> m_shapes = {};
	protected void CreateDebugCircle(vector position, int color = Color.RED, bool clear = false)
	{
		if(clear)
			m_shapes.Clear();
		
		m_shapes.Insert(Shape.CreateSphere(color, ShapeFlags.DEFAULT, position, 0.2));
	}
}
