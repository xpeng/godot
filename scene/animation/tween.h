/*************************************************************************/
/*  tween.h                                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef TWEEN_H
#define TWEEN_H

#include "scene/main/node.h"


class Tween : public Node {

	OBJ_TYPE( Tween, Node );

public:
	enum TweenProcessMode {
		TWEEN_PROCESS_FIXED,
		TWEEN_PROCESS_IDLE,
	};

    enum TransitionType {
        TRANS_LINEAR,
        TRANS_SINE,
        TRANS_QUINT,
        TRANS_QUART,
        TRANS_QUAD,
        TRANS_EXPO,
        TRANS_ELASTIC,
        TRANS_CUBIC,
        TRANS_CIRC,
        TRANS_BOUNCE,
        TRANS_BACK,

		TRANS_COUNT,
    };
        
    enum EaseType {
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT,
		EASE_OUT_IN,

		EASE_COUNT,
    };

private:

	struct InterpolateData {
		bool active;
		bool is_method;
		real_t elapsed;

		Variant object;
		String key;
		Variant initial_val;
		Variant delta_val;
		Variant final_val;
		real_t times_in_sec;
		TransitionType trans_type;
		EaseType ease_type;
	};

	String autoplay;
	TweenProcessMode tween_process_mode;
	bool processing;
	bool active;
	bool repeat;
	float speed_scale;

	List<InterpolateData> interpolates;

	typedef real_t (*interpolater)(real_t t, real_t b, real_t c, real_t d);
	static interpolater interpolaters[TRANS_COUNT][EASE_COUNT];

	real_t _run_equation(TransitionType p_trans_type, EaseType p_ease_type, real_t t, real_t b, real_t c, real_t d);
	Variant _run_equation(InterpolateData& p_data);
	bool _calc_delta_val(InterpolateData& p_data);
	bool _apply_tween_value(InterpolateData& p_data, Variant& value);

	void _tween_process(float p_delta);
	void _set_process(bool p_process,bool p_force=false);

protected:

	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name,Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

	static void _bind_methods();

public:

	bool is_active() const;
	void set_active(bool p_active);

	bool is_repeat() const;
	void set_repeat(bool p_repeat);

	void set_tween_process_mode(TweenProcessMode p_mode);
	TweenProcessMode get_tween_process_mode() const;

	void set_speed(float p_speed);
	float get_speed() const;

	bool start();
	bool reset(Variant p_object, String p_key);
	bool reset_all();
	bool stop(Variant p_object, String p_key);
	bool stop_all();
	bool resume(Variant p_object, String p_key);
	bool resume_all();
	bool remove(Variant p_object, String p_key);
	bool remove_all();

	bool seek(real_t p_time);

	bool interpolate_property(Variant p_object
		, String p_property
		, Variant p_initial_val
		, Variant p_final_val
		, real_t p_times_in_sec
		, TransitionType p_trans_type
		, EaseType p_ease_type
	);

	bool interpolate_method(Variant p_object
		, String p_method
		, Variant p_initial_val
		, Variant p_final_val
		, real_t p_times_in_sec
		, TransitionType p_trans_type
		, EaseType p_ease_type
	);

	Tween();
	~Tween();
};

VARIANT_ENUM_CAST( Tween::TweenProcessMode );
VARIANT_ENUM_CAST( Tween::TransitionType );
VARIANT_ENUM_CAST( Tween::EaseType );

#endif

