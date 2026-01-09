class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression")]
	protected float m_fMaxRange;
	
	[Attribute("1", UIWidgets.Auto, "Distance range in meters for a bullet to apply flinch")]
	protected float m_fFlinchRange;
	
	[Attribute("50", UIWidgets.Auto, "Distance up to which cover is recognized (meters)")]
	protected float m_fCoverTraceLength;
	
	[Attribute("5.0", UIWidgets.Auto, "Seconds of no new suppression before recovery starts")]
	protected float m_fRecoveryDelay;
	
	[Attribute("0.1", UIWidgets.Auto, "Suppression recovery speed per second")]
	protected float m_fRecoveryRate;
	
	[Attribute("1", UIWidgets.Auto, "Multiplier for the visual intensity of the suppression effect")]
	protected float m_fEffectIntensity;

	[Attribute("2.0", UIWidgets.Auto, "Multiplier applied to projectile mass for suppression")]
	protected float m_fMassMultiplier;
	
	[Attribute("0.001", UIWidgets.Auto, "Multiplier applied to projectile speed for suppression")]
	protected float m_fSpeedMultiplier;
	
	[Attribute("0.075", UIWidgets.Auto, "Global suppression multiplier per bullet")]
	protected float m_fBaseSuppressionMultiplier;
	
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
				vector flybyPos = GetFlybyPoint(projPos, projectile.position, playerEyePos);
				float dist = vector.Distance(flybyPos, playerEyePos);
				
				if (dist <= m_fMaxRange)
				{
					// Cover check should move to seperate method so it can be reused for hits around a player
					// "perfect shot" projectile trace towards player head to check whether it would have been in some kind of cover
					TraceParam tp = MakeTraceParam(playerEyePos - projectile.move.GetVelocity().Normalized() * m_fCoverTraceLength, playerEyePos, TraceFlags.ENTS | TraceFlags.WORLD);
					tp.Exclude = player;
					float trace = GetWorld().TraceMove(tp);
					CreateDebugCircle(flybyPos, Color.GREEN);
					AddSuppression(projectile, dist, m_fCoverTraceLength * trace > m_fCoverTraceLength - 0.1);
				}
				else
				{
					CreateDebugCircle(flybyPos, Color.PINK);
				}
				
				if (dist <= m_fFlinchRange)
					Flinch();

		        m_aProjectiles.Remove(i);
		        continue;
		    }
			else
				CreateDebugCircle(projPos);
		
			projectile.position = projPos;
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
	}
	
	protected void UpdateEffect()
	{
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if(sse)
			sse.UpdateSuppresion();
	}
	
	protected void Flinch()
	{
		//Aim effect. kind of wonkie
		//IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		//CharacterControllerComponent cc = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
		//vector dir = "45 45 0";
		//cc.GetInputContext().SetAimingAngles( dir );
		//cc.GetInputContext().SetHeadingAngle( 45 );

		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if(sse)
			sse.Flinch();
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
		projComp.position = muzzle.GetOwner().GetOrigin();
		
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

	vector GetFlybyPoint(vector currPos, vector prevPos, vector playerPos)
	{
		vector seg = currPos - prevPos;
		float segLenSq = vector.DistanceSq(prevPos, currPos);
	
		if (segLenSq <= 0.000001)
			return prevPos;
	
		vector toPlayer = playerPos - prevPos;
	
		float t = vector.Dot(toPlayer, seg) / segLenSq;
		t = Math.Clamp(t, 0.0, 1.0);
	
		vector closestPoint = prevPos + seg * t;
		
		return closestPoint;
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
