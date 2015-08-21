
extends Node2D

# member variables here, example:
# var a=2
# var b="textvar"

func _ready():
	# Initialization here
	var names = get_node("hero").get_animation_names()
	print(names)
	pass
	
	for c in get_children():
		if not c extends Button:
			continue
		c.connect("pressed", self, "_on_button_pressed", [c.get_name()])

func _on_button_pressed(what):
	var hero = get_node("hero")
	var hero_mesh = get_node("hero_mesh")
	
	if what == "random":
		var names = hero.get_animation_names()
		hero.play(names[randi() % names.size()], 1, true)
		var names = hero_mesh.get_animation_names()
		hero_mesh.play(names[randi() % names.size()], 1, true)
	
	elif what == "preload":
		var preload_set = get_node("preload_set")
		hero.set_avatar_path(preload_set.get_path())
		hero_mesh.set_avatar_path(preload_set.get_path())
	
	elif what == "script":
		var sets = [
			["body", preload("res://spine/hero/skin/body.png")],
			["cape", preload("res://spine/hero/skin/cape.png")],
			["head", preload("res://spine/hero/skin/head.png")],
			["sword", preload("res://spine/hero/skin/sword.png")],
		]
		var preloader = get_node("script_set")
		preloader.remove_all()
		
		for set in sets:
			if rand_range(0, 99) < 50:
				var name = set[0]
				var tex = set[1]
				preloader.add_resource(name, tex)
		
		hero.set_avatar_path(preloader.get_path())
		hero_mesh.set_avatar_path(preloader.get_path())
		
	elif what == "reset":
		hero.set_avatar_path("")
		hero_mesh.set_avatar_path("")
	


