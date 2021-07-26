
(function(l, r) { if (l.getElementById('livereloadscript')) return; r = l.createElement('script'); r.async = 1; r.src = '//' + (window.location.host || 'localhost').split(':')[0] + ':35729/livereload.js?snipver=1'; r.id = 'livereloadscript'; l.getElementsByTagName('head')[0].appendChild(r) })(window.document);
var app = (function () {
    'use strict';

    function noop() { }
    const identity = x => x;
    function assign(tar, src) {
        // @ts-ignore
        for (const k in src)
            tar[k] = src[k];
        return tar;
    }
    function add_location(element, file, line, column, char) {
        element.__svelte_meta = {
            loc: { file, line, column, char }
        };
    }
    function run(fn) {
        return fn();
    }
    function blank_object() {
        return Object.create(null);
    }
    function run_all(fns) {
        fns.forEach(run);
    }
    function is_function(thing) {
        return typeof thing === 'function';
    }
    function safe_not_equal(a, b) {
        return a != a ? b == b : a !== b || ((a && typeof a === 'object') || typeof a === 'function');
    }
    function is_empty(obj) {
        return Object.keys(obj).length === 0;
    }
    function validate_store(store, name) {
        if (store != null && typeof store.subscribe !== 'function') {
            throw new Error(`'${name}' is not a store with a 'subscribe' method`);
        }
    }
    function subscribe(store, ...callbacks) {
        if (store == null) {
            return noop;
        }
        const unsub = store.subscribe(...callbacks);
        return unsub.unsubscribe ? () => unsub.unsubscribe() : unsub;
    }
    function component_subscribe(component, store, callback) {
        component.$$.on_destroy.push(subscribe(store, callback));
    }
    function create_slot(definition, ctx, $$scope, fn) {
        if (definition) {
            const slot_ctx = get_slot_context(definition, ctx, $$scope, fn);
            return definition[0](slot_ctx);
        }
    }
    function get_slot_context(definition, ctx, $$scope, fn) {
        return definition[1] && fn
            ? assign($$scope.ctx.slice(), definition[1](fn(ctx)))
            : $$scope.ctx;
    }
    function get_slot_changes(definition, $$scope, dirty, fn) {
        if (definition[2] && fn) {
            const lets = definition[2](fn(dirty));
            if ($$scope.dirty === undefined) {
                return lets;
            }
            if (typeof lets === 'object') {
                const merged = [];
                const len = Math.max($$scope.dirty.length, lets.length);
                for (let i = 0; i < len; i += 1) {
                    merged[i] = $$scope.dirty[i] | lets[i];
                }
                return merged;
            }
            return $$scope.dirty | lets;
        }
        return $$scope.dirty;
    }
    function update_slot(slot, slot_definition, ctx, $$scope, dirty, get_slot_changes_fn, get_slot_context_fn) {
        const slot_changes = get_slot_changes(slot_definition, $$scope, dirty, get_slot_changes_fn);
        if (slot_changes) {
            const slot_context = get_slot_context(slot_definition, ctx, $$scope, get_slot_context_fn);
            slot.p(slot_context, slot_changes);
        }
    }

    const is_client = typeof window !== 'undefined';
    let now = is_client
        ? () => window.performance.now()
        : () => Date.now();
    let raf = is_client ? cb => requestAnimationFrame(cb) : noop;

    const tasks = new Set();
    function run_tasks(now) {
        tasks.forEach(task => {
            if (!task.c(now)) {
                tasks.delete(task);
                task.f();
            }
        });
        if (tasks.size !== 0)
            raf(run_tasks);
    }
    /**
     * Creates a new task that runs on each raf frame
     * until it returns a falsy value or is aborted
     */
    function loop(callback) {
        let task;
        if (tasks.size === 0)
            raf(run_tasks);
        return {
            promise: new Promise(fulfill => {
                tasks.add(task = { c: callback, f: fulfill });
            }),
            abort() {
                tasks.delete(task);
            }
        };
    }

    function append(target, node) {
        target.appendChild(node);
    }
    function insert(target, node, anchor) {
        target.insertBefore(node, anchor || null);
    }
    function detach(node) {
        node.parentNode.removeChild(node);
    }
    function element(name) {
        return document.createElement(name);
    }
    function text(data) {
        return document.createTextNode(data);
    }
    function space() {
        return text(' ');
    }
    function empty() {
        return text('');
    }
    function listen(node, event, handler, options) {
        node.addEventListener(event, handler, options);
        return () => node.removeEventListener(event, handler, options);
    }
    function attr(node, attribute, value) {
        if (value == null)
            node.removeAttribute(attribute);
        else if (node.getAttribute(attribute) !== value)
            node.setAttribute(attribute, value);
    }
    function children(element) {
        return Array.from(element.childNodes);
    }
    function set_input_value(input, value) {
        input.value = value == null ? '' : value;
    }
    function custom_event(type, detail) {
        const e = document.createEvent('CustomEvent');
        e.initCustomEvent(type, false, false, detail);
        return e;
    }

    const active_docs = new Set();
    let active = 0;
    // https://github.com/darkskyapp/string-hash/blob/master/index.js
    function hash(str) {
        let hash = 5381;
        let i = str.length;
        while (i--)
            hash = ((hash << 5) - hash) ^ str.charCodeAt(i);
        return hash >>> 0;
    }
    function create_rule(node, a, b, duration, delay, ease, fn, uid = 0) {
        const step = 16.666 / duration;
        let keyframes = '{\n';
        for (let p = 0; p <= 1; p += step) {
            const t = a + (b - a) * ease(p);
            keyframes += p * 100 + `%{${fn(t, 1 - t)}}\n`;
        }
        const rule = keyframes + `100% {${fn(b, 1 - b)}}\n}`;
        const name = `__svelte_${hash(rule)}_${uid}`;
        const doc = node.ownerDocument;
        active_docs.add(doc);
        const stylesheet = doc.__svelte_stylesheet || (doc.__svelte_stylesheet = doc.head.appendChild(element('style')).sheet);
        const current_rules = doc.__svelte_rules || (doc.__svelte_rules = {});
        if (!current_rules[name]) {
            current_rules[name] = true;
            stylesheet.insertRule(`@keyframes ${name} ${rule}`, stylesheet.cssRules.length);
        }
        const animation = node.style.animation || '';
        node.style.animation = `${animation ? `${animation}, ` : ''}${name} ${duration}ms linear ${delay}ms 1 both`;
        active += 1;
        return name;
    }
    function delete_rule(node, name) {
        const previous = (node.style.animation || '').split(', ');
        const next = previous.filter(name
            ? anim => anim.indexOf(name) < 0 // remove specific animation
            : anim => anim.indexOf('__svelte') === -1 // remove all Svelte animations
        );
        const deleted = previous.length - next.length;
        if (deleted) {
            node.style.animation = next.join(', ');
            active -= deleted;
            if (!active)
                clear_rules();
        }
    }
    function clear_rules() {
        raf(() => {
            if (active)
                return;
            active_docs.forEach(doc => {
                const stylesheet = doc.__svelte_stylesheet;
                let i = stylesheet.cssRules.length;
                while (i--)
                    stylesheet.deleteRule(i);
                doc.__svelte_rules = {};
            });
            active_docs.clear();
        });
    }

    function create_animation(node, from, fn, params) {
        if (!from)
            return noop;
        const to = node.getBoundingClientRect();
        if (from.left === to.left && from.right === to.right && from.top === to.top && from.bottom === to.bottom)
            return noop;
        const { delay = 0, duration = 300, easing = identity, 
        // @ts-ignore todo: should this be separated from destructuring? Or start/end added to public api and documentation?
        start: start_time = now() + delay, 
        // @ts-ignore todo:
        end = start_time + duration, tick = noop, css } = fn(node, { from, to }, params);
        let running = true;
        let started = false;
        let name;
        function start() {
            if (css) {
                name = create_rule(node, 0, 1, duration, delay, easing, css);
            }
            if (!delay) {
                started = true;
            }
        }
        function stop() {
            if (css)
                delete_rule(node, name);
            running = false;
        }
        loop(now => {
            if (!started && now >= start_time) {
                started = true;
            }
            if (started && now >= end) {
                tick(1, 0);
                stop();
            }
            if (!running) {
                return false;
            }
            if (started) {
                const p = now - start_time;
                const t = 0 + 1 * easing(p / duration);
                tick(t, 1 - t);
            }
            return true;
        });
        start();
        tick(0, 1);
        return stop;
    }
    function fix_position(node) {
        const style = getComputedStyle(node);
        if (style.position !== 'absolute' && style.position !== 'fixed') {
            const { width, height } = style;
            const a = node.getBoundingClientRect();
            node.style.position = 'absolute';
            node.style.width = width;
            node.style.height = height;
            add_transform(node, a);
        }
    }
    function add_transform(node, a) {
        const b = node.getBoundingClientRect();
        if (a.left !== b.left || a.top !== b.top) {
            const style = getComputedStyle(node);
            const transform = style.transform === 'none' ? '' : style.transform;
            node.style.transform = `${transform} translate(${a.left - b.left}px, ${a.top - b.top}px)`;
        }
    }

    let current_component;
    function set_current_component(component) {
        current_component = component;
    }
    function get_current_component() {
        if (!current_component)
            throw new Error('Function called outside component initialization');
        return current_component;
    }
    function onMount(fn) {
        get_current_component().$$.on_mount.push(fn);
    }
    function setContext(key, context) {
        get_current_component().$$.context.set(key, context);
    }
    function getContext(key) {
        return get_current_component().$$.context.get(key);
    }

    const dirty_components = [];
    const binding_callbacks = [];
    const render_callbacks = [];
    const flush_callbacks = [];
    const resolved_promise = Promise.resolve();
    let update_scheduled = false;
    function schedule_update() {
        if (!update_scheduled) {
            update_scheduled = true;
            resolved_promise.then(flush);
        }
    }
    function tick() {
        schedule_update();
        return resolved_promise;
    }
    function add_render_callback(fn) {
        render_callbacks.push(fn);
    }
    let flushing = false;
    const seen_callbacks = new Set();
    function flush() {
        if (flushing)
            return;
        flushing = true;
        do {
            // first, call beforeUpdate functions
            // and update components
            for (let i = 0; i < dirty_components.length; i += 1) {
                const component = dirty_components[i];
                set_current_component(component);
                update(component.$$);
            }
            set_current_component(null);
            dirty_components.length = 0;
            while (binding_callbacks.length)
                binding_callbacks.pop()();
            // then, once components are updated, call
            // afterUpdate functions. This may cause
            // subsequent updates...
            for (let i = 0; i < render_callbacks.length; i += 1) {
                const callback = render_callbacks[i];
                if (!seen_callbacks.has(callback)) {
                    // ...so guard against infinite loops
                    seen_callbacks.add(callback);
                    callback();
                }
            }
            render_callbacks.length = 0;
        } while (dirty_components.length);
        while (flush_callbacks.length) {
            flush_callbacks.pop()();
        }
        update_scheduled = false;
        flushing = false;
        seen_callbacks.clear();
    }
    function update($$) {
        if ($$.fragment !== null) {
            $$.update();
            run_all($$.before_update);
            const dirty = $$.dirty;
            $$.dirty = [-1];
            $$.fragment && $$.fragment.p($$.ctx, dirty);
            $$.after_update.forEach(add_render_callback);
        }
    }

    let promise;
    function wait() {
        if (!promise) {
            promise = Promise.resolve();
            promise.then(() => {
                promise = null;
            });
        }
        return promise;
    }
    function dispatch(node, direction, kind) {
        node.dispatchEvent(custom_event(`${direction ? 'intro' : 'outro'}${kind}`));
    }
    const outroing = new Set();
    let outros;
    function group_outros() {
        outros = {
            r: 0,
            c: [],
            p: outros // parent group
        };
    }
    function check_outros() {
        if (!outros.r) {
            run_all(outros.c);
        }
        outros = outros.p;
    }
    function transition_in(block, local) {
        if (block && block.i) {
            outroing.delete(block);
            block.i(local);
        }
    }
    function transition_out(block, local, detach, callback) {
        if (block && block.o) {
            if (outroing.has(block))
                return;
            outroing.add(block);
            outros.c.push(() => {
                outroing.delete(block);
                if (callback) {
                    if (detach)
                        block.d(1);
                    callback();
                }
            });
            block.o(local);
        }
    }
    const null_transition = { duration: 0 };
    function create_in_transition(node, fn, params) {
        let config = fn(node, params);
        let running = false;
        let animation_name;
        let task;
        let uid = 0;
        function cleanup() {
            if (animation_name)
                delete_rule(node, animation_name);
        }
        function go() {
            const { delay = 0, duration = 300, easing = identity, tick = noop, css } = config || null_transition;
            if (css)
                animation_name = create_rule(node, 0, 1, duration, delay, easing, css, uid++);
            tick(0, 1);
            const start_time = now() + delay;
            const end_time = start_time + duration;
            if (task)
                task.abort();
            running = true;
            add_render_callback(() => dispatch(node, true, 'start'));
            task = loop(now => {
                if (running) {
                    if (now >= end_time) {
                        tick(1, 0);
                        dispatch(node, true, 'end');
                        cleanup();
                        return running = false;
                    }
                    if (now >= start_time) {
                        const t = easing((now - start_time) / duration);
                        tick(t, 1 - t);
                    }
                }
                return running;
            });
        }
        let started = false;
        return {
            start() {
                if (started)
                    return;
                delete_rule(node);
                if (is_function(config)) {
                    config = config();
                    wait().then(go);
                }
                else {
                    go();
                }
            },
            invalidate() {
                started = false;
            },
            end() {
                if (running) {
                    cleanup();
                    running = false;
                }
            }
        };
    }
    function create_out_transition(node, fn, params) {
        let config = fn(node, params);
        let running = true;
        let animation_name;
        const group = outros;
        group.r += 1;
        function go() {
            const { delay = 0, duration = 300, easing = identity, tick = noop, css } = config || null_transition;
            if (css)
                animation_name = create_rule(node, 1, 0, duration, delay, easing, css);
            const start_time = now() + delay;
            const end_time = start_time + duration;
            add_render_callback(() => dispatch(node, false, 'start'));
            loop(now => {
                if (running) {
                    if (now >= end_time) {
                        tick(0, 1);
                        dispatch(node, false, 'end');
                        if (!--group.r) {
                            // this will result in `end()` being called,
                            // so we don't need to clean up here
                            run_all(group.c);
                        }
                        return false;
                    }
                    if (now >= start_time) {
                        const t = easing((now - start_time) / duration);
                        tick(1 - t, t);
                    }
                }
                return running;
            });
        }
        if (is_function(config)) {
            wait().then(() => {
                // @ts-ignore
                config = config();
                go();
            });
        }
        else {
            go();
        }
        return {
            end(reset) {
                if (reset && config.tick) {
                    config.tick(1, 0);
                }
                if (running) {
                    if (animation_name)
                        delete_rule(node, animation_name);
                    running = false;
                }
            }
        };
    }

    const globals = (typeof window !== 'undefined'
        ? window
        : typeof globalThis !== 'undefined'
            ? globalThis
            : global);
    function outro_and_destroy_block(block, lookup) {
        transition_out(block, 1, 1, () => {
            lookup.delete(block.key);
        });
    }
    function fix_and_outro_and_destroy_block(block, lookup) {
        block.f();
        outro_and_destroy_block(block, lookup);
    }
    function update_keyed_each(old_blocks, dirty, get_key, dynamic, ctx, list, lookup, node, destroy, create_each_block, next, get_context) {
        let o = old_blocks.length;
        let n = list.length;
        let i = o;
        const old_indexes = {};
        while (i--)
            old_indexes[old_blocks[i].key] = i;
        const new_blocks = [];
        const new_lookup = new Map();
        const deltas = new Map();
        i = n;
        while (i--) {
            const child_ctx = get_context(ctx, list, i);
            const key = get_key(child_ctx);
            let block = lookup.get(key);
            if (!block) {
                block = create_each_block(key, child_ctx);
                block.c();
            }
            else if (dynamic) {
                block.p(child_ctx, dirty);
            }
            new_lookup.set(key, new_blocks[i] = block);
            if (key in old_indexes)
                deltas.set(key, Math.abs(i - old_indexes[key]));
        }
        const will_move = new Set();
        const did_move = new Set();
        function insert(block) {
            transition_in(block, 1);
            block.m(node, next);
            lookup.set(block.key, block);
            next = block.first;
            n--;
        }
        while (o && n) {
            const new_block = new_blocks[n - 1];
            const old_block = old_blocks[o - 1];
            const new_key = new_block.key;
            const old_key = old_block.key;
            if (new_block === old_block) {
                // do nothing
                next = new_block.first;
                o--;
                n--;
            }
            else if (!new_lookup.has(old_key)) {
                // remove old block
                destroy(old_block, lookup);
                o--;
            }
            else if (!lookup.has(new_key) || will_move.has(new_key)) {
                insert(new_block);
            }
            else if (did_move.has(old_key)) {
                o--;
            }
            else if (deltas.get(new_key) > deltas.get(old_key)) {
                did_move.add(new_key);
                insert(new_block);
            }
            else {
                will_move.add(old_key);
                o--;
            }
        }
        while (o--) {
            const old_block = old_blocks[o];
            if (!new_lookup.has(old_block.key))
                destroy(old_block, lookup);
        }
        while (n)
            insert(new_blocks[n - 1]);
        return new_blocks;
    }
    function validate_each_keys(ctx, list, get_context, get_key) {
        const keys = new Set();
        for (let i = 0; i < list.length; i++) {
            const key = get_key(get_context(ctx, list, i));
            if (keys.has(key)) {
                throw new Error('Cannot have duplicate keys in a keyed each');
            }
            keys.add(key);
        }
    }
    function create_component(block) {
        block && block.c();
    }
    function mount_component(component, target, anchor) {
        const { fragment, on_mount, on_destroy, after_update } = component.$$;
        fragment && fragment.m(target, anchor);
        // onMount happens before the initial afterUpdate
        add_render_callback(() => {
            const new_on_destroy = on_mount.map(run).filter(is_function);
            if (on_destroy) {
                on_destroy.push(...new_on_destroy);
            }
            else {
                // Edge case - component was destroyed immediately,
                // most likely as a result of a binding initialising
                run_all(new_on_destroy);
            }
            component.$$.on_mount = [];
        });
        after_update.forEach(add_render_callback);
    }
    function destroy_component(component, detaching) {
        const $$ = component.$$;
        if ($$.fragment !== null) {
            run_all($$.on_destroy);
            $$.fragment && $$.fragment.d(detaching);
            // TODO null out other refs, including component.$$ (but need to
            // preserve final state?)
            $$.on_destroy = $$.fragment = null;
            $$.ctx = [];
        }
    }
    function make_dirty(component, i) {
        if (component.$$.dirty[0] === -1) {
            dirty_components.push(component);
            schedule_update();
            component.$$.dirty.fill(0);
        }
        component.$$.dirty[(i / 31) | 0] |= (1 << (i % 31));
    }
    function init(component, options, instance, create_fragment, not_equal, props, dirty = [-1]) {
        const parent_component = current_component;
        set_current_component(component);
        const $$ = component.$$ = {
            fragment: null,
            ctx: null,
            // state
            props,
            update: noop,
            not_equal,
            bound: blank_object(),
            // lifecycle
            on_mount: [],
            on_destroy: [],
            before_update: [],
            after_update: [],
            context: new Map(parent_component ? parent_component.$$.context : []),
            // everything else
            callbacks: blank_object(),
            dirty,
            skip_bound: false
        };
        let ready = false;
        $$.ctx = instance
            ? instance(component, options.props || {}, (i, ret, ...rest) => {
                const value = rest.length ? rest[0] : ret;
                if ($$.ctx && not_equal($$.ctx[i], $$.ctx[i] = value)) {
                    if (!$$.skip_bound && $$.bound[i])
                        $$.bound[i](value);
                    if (ready)
                        make_dirty(component, i);
                }
                return ret;
            })
            : [];
        $$.update();
        ready = true;
        run_all($$.before_update);
        // `false` as a special case of no DOM component
        $$.fragment = create_fragment ? create_fragment($$.ctx) : false;
        if (options.target) {
            if (options.hydrate) {
                const nodes = children(options.target);
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.l(nodes);
                nodes.forEach(detach);
            }
            else {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.c();
            }
            if (options.intro)
                transition_in(component.$$.fragment);
            mount_component(component, options.target, options.anchor);
            flush();
        }
        set_current_component(parent_component);
    }
    /**
     * Base class for Svelte components. Used when dev=false.
     */
    class SvelteComponent {
        $destroy() {
            destroy_component(this, 1);
            this.$destroy = noop;
        }
        $on(type, callback) {
            const callbacks = (this.$$.callbacks[type] || (this.$$.callbacks[type] = []));
            callbacks.push(callback);
            return () => {
                const index = callbacks.indexOf(callback);
                if (index !== -1)
                    callbacks.splice(index, 1);
            };
        }
        $set($$props) {
            if (this.$$set && !is_empty($$props)) {
                this.$$.skip_bound = true;
                this.$$set($$props);
                this.$$.skip_bound = false;
            }
        }
    }

    function dispatch_dev(type, detail) {
        document.dispatchEvent(custom_event(type, Object.assign({ version: '3.32.3' }, detail)));
    }
    function append_dev(target, node) {
        dispatch_dev('SvelteDOMInsert', { target, node });
        append(target, node);
    }
    function insert_dev(target, node, anchor) {
        dispatch_dev('SvelteDOMInsert', { target, node, anchor });
        insert(target, node, anchor);
    }
    function detach_dev(node) {
        dispatch_dev('SvelteDOMRemove', { node });
        detach(node);
    }
    function listen_dev(node, event, handler, options, has_prevent_default, has_stop_propagation) {
        const modifiers = options === true ? ['capture'] : options ? Array.from(Object.keys(options)) : [];
        if (has_prevent_default)
            modifiers.push('preventDefault');
        if (has_stop_propagation)
            modifiers.push('stopPropagation');
        dispatch_dev('SvelteDOMAddEventListener', { node, event, handler, modifiers });
        const dispose = listen(node, event, handler, options);
        return () => {
            dispatch_dev('SvelteDOMRemoveEventListener', { node, event, handler, modifiers });
            dispose();
        };
    }
    function attr_dev(node, attribute, value) {
        attr(node, attribute, value);
        if (value == null)
            dispatch_dev('SvelteDOMRemoveAttribute', { node, attribute });
        else
            dispatch_dev('SvelteDOMSetAttribute', { node, attribute, value });
    }
    function prop_dev(node, property, value) {
        node[property] = value;
        dispatch_dev('SvelteDOMSetProperty', { node, property, value });
    }
    function set_data_dev(text, data) {
        data = '' + data;
        if (text.wholeText === data)
            return;
        dispatch_dev('SvelteDOMSetData', { node: text, data });
        text.data = data;
    }
    function validate_each_argument(arg) {
        if (typeof arg !== 'string' && !(arg && typeof arg === 'object' && 'length' in arg)) {
            let msg = '{#each} only iterates over array-like objects.';
            if (typeof Symbol === 'function' && arg && Symbol.iterator in arg) {
                msg += ' You can use a spread to convert this iterable into an array.';
            }
            throw new Error(msg);
        }
    }
    function validate_slots(name, slot, keys) {
        for (const slot_key of Object.keys(slot)) {
            if (!~keys.indexOf(slot_key)) {
                console.warn(`<${name}> received an unexpected slot "${slot_key}".`);
            }
        }
    }
    /**
     * Base class for Svelte components with some minor dev-enhancements. Used when dev=true.
     */
    class SvelteComponentDev extends SvelteComponent {
        constructor(options) {
            if (!options || (!options.target && !options.$$inline)) {
                throw new Error("'target' is a required option");
            }
            super();
        }
        $destroy() {
            super.$destroy();
            this.$destroy = () => {
                console.warn('Component was already destroyed'); // eslint-disable-line no-console
            };
        }
        $capture_state() { }
        $inject_state() { }
    }

    const subscriber_queue = [];
    /**
     * Create a `Writable` store that allows both updating and reading by subscription.
     * @param {*=}value initial value
     * @param {StartStopNotifier=}start start and stop notifications for subscriptions
     */
    function writable(value, start = noop) {
        let stop;
        const subscribers = [];
        function set(new_value) {
            if (safe_not_equal(value, new_value)) {
                value = new_value;
                if (stop) { // store is ready
                    const run_queue = !subscriber_queue.length;
                    for (let i = 0; i < subscribers.length; i += 1) {
                        const s = subscribers[i];
                        s[1]();
                        subscriber_queue.push(s, value);
                    }
                    if (run_queue) {
                        for (let i = 0; i < subscriber_queue.length; i += 2) {
                            subscriber_queue[i][0](subscriber_queue[i + 1]);
                        }
                        subscriber_queue.length = 0;
                    }
                }
            }
        }
        function update(fn) {
            set(fn(value));
        }
        function subscribe(run, invalidate = noop) {
            const subscriber = [run, invalidate];
            subscribers.push(subscriber);
            if (subscribers.length === 1) {
                stop = start(set) || noop;
            }
            run(value);
            return () => {
                const index = subscribers.indexOf(subscriber);
                if (index !== -1) {
                    subscribers.splice(index, 1);
                }
                if (subscribers.length === 0) {
                    stop();
                    stop = null;
                }
            };
        }
        return { set, update, subscribe };
    }

    function d(e,r=!1){return e=e.slice(e.startsWith("/#")?2:0,e.endsWith("/*")?-2:void 0),e.startsWith("/")||(e="/"+e),e==="/"&&(e=""),r&&!e.endsWith("/")&&(e+="/"),e}function b(e,r){e=d(e,!0),r=d(r,!0);let a=[],n={},t=!0,o=e.split("/").map(c=>c.startsWith(":")?(a.push(c.slice(1)),"([^\\/]+)"):c).join("\\/"),i=r.match(new RegExp(`^${o}$`));return i||(t=!1,i=r.match(new RegExp(`^${o}`))),i?(a.forEach((c,w)=>n[c]=i[w+1]),{exact:t,params:n,part:i[0].slice(0,-1)}):null}function x(e,r,a){if(a==="")return e;if(a[0]==="/")return a;let n=i=>i.split("/").filter(c=>c!==""),t=n(e),o=r?n(r):[];return "/"+o.map((i,c)=>t[c]).join("/")+"/"+a}function h(e,r,a,n){let t=[r,"data-"+r].reduce((o,i)=>{let c=e.getAttribute(i);return a&&e.removeAttribute(i),c===null?o:c},!1);return !n&&t===""?!0:t||n||!1}function v(e){let r=e.split("&").map(a=>a.split("=")).reduce((a,n)=>{let t=n[0];if(!t)return a;let o=n.length>1?n[n.length-1]:!0;return typeof o=="string"&&o.includes(",")&&(o=o.split(",")),a[t]===void 0?a[t]=[o]:a[t].push(o),a},{});return Object.entries(r).reduce((a,n)=>(a[n[0]]=n[1].length>1?n[1]:n[1][0],a),{})}function y(e){throw new Error("[Tinro] "+e)}var k=1,S=2,M=3,H=4;function j(e,r,a,n){return e===k?r&&r():e===S?a&&a():n&&n()}function C(){return !window||window.location.pathname==="srcdoc"?M:k}var s={HISTORY:k,HASH:S,MEMORY:M,OFF:H,run:j,getDeafault:C};var O,R,l=W();function W(){let e=s.getDeafault(),r,a=o=>window.onhashchange=window.onpopstate=R=null,n=o=>r&&r(_(e));function t(o){o&&(e=o),a(),e!==s.OFF&&s.run(e,i=>window.onpopstate=n,i=>window.onhashchange=n)&&n();}return {mode:o=>t(o),get:o=>_(e),go(o){z(e,o),n();},start(o){r=o,t();},stop(){r=null,t(s.OFF);}}}function z(e,r){s.run(e,a=>history.pushState({},"",r),a=>window.location.hash=r,a=>R=r);}function _(e){let r=O,a=O=s.run(e,t=>window.location.pathname+window.location.search,t=>String(window.location.hash.slice(1)||"/"),t=>R||"/"),n=a.match(/^([^?#]+)(?:\?([^#]+))?(?:\#(.+))?$/);return {url:a,from:r,path:n[1]||"",query:v(n[2]||""),hash:n[3]||""}}function $(e){let r=getContext("tinro");r&&(r.exact||r.fallback)&&y(`${e.fallback?"<Route fallback>":`<Route path="${e.path}">`}  can't be inside ${r.fallback?"<Route fallback>":`<Route path="${r.path||"/"}"> with exact path`}`);let a=e.fallback?"fallbacks":"childs",n=writable({}),t={router:{},exact:!1,pattern:null,meta:{},parent:r,fallback:e.fallback,redirect:!1,firstmatch:!1,breadcrumb:null,matched:!1,childs:new Set,activeChilds:new Set,fallbacks:new Set,update(o){t.exact=!o.path.endsWith("/*"),t.pattern=d(`${t.parent&&t.parent.pattern||""}${o.path}`),t.redirect=o.redirect,t.firstmatch=o.firstmatch,t.breadcrumb=o.breadcrumb,t.match();},register:()=>{if(!!t.parent)return t.parent[a].add(t),()=>{t.parent[a].delete(t),t.router.un&&t.router.un();}},show:()=>{e.onShow(),!t.fallback&&t.parent&&t.parent.activeChilds.add(t);},hide:()=>{e.onHide(),!t.fallback&&t.parent&&t.parent.activeChilds.delete(t);},match:async()=>{t.matched=!1;let{path:o,url:i,from:c,query:w}=t.router,u=b(t.pattern,o);if(!t.fallback&&u&&t.redirect&&(!t.exact||t.exact&&u.exact))return await tick(),m.goto(x(o,t.parent&&t.parent.pattern,t.redirect));if(t.meta=u&&{from:c,url:i,query:w,match:u.part,pattern:t.pattern,breadcrumbs:t.parent&&t.parent.meta&&t.parent.meta.breadcrumbs.slice()||[],params:u.params,subscribe:n.subscribe},t.breadcrumb&&t.meta&&t.meta.breadcrumbs.push({name:t.breadcrumb,path:u.part}),n.set(t.meta),u&&!t.fallback&&(!t.exact||t.exact&&u.exact)&&(!t.parent||!t.parent.firstmatch||!t.parent.matched)?(e.onMeta(t.meta),t.parent&&(t.parent.matched=!0),t.show()):t.hide(),await tick(),u&&!t.fallback&&(t.childs.size>0&&t.activeChilds.size==0||t.childs.size==0&&t.fallbacks.size>0)){let f=t;for(;f.fallbacks.size==0;)if(f=f.parent,!f)return;f&&f.fallbacks.forEach(p=>{p.redirect?m.goto(x("/",p.parent&&p.parent.pattern,p.redirect)):p.show();});}}};return setContext("tinro",t),onMount(()=>t.register()),t.router.un=m.subscribe(o=>{t.router.path=o.path,t.router.url=o.url,t.router.query=o.query,t.router.from=o.from,t.pattern!==null&&t.match();}),t}function g(){return getContext("tinro").meta}var m=K();function K(){let{subscribe:e}=writable(l.get(),r=>{l.start(r);let a=T(l.go);return ()=>{l.stop(),a();}});return {subscribe:e,goto:r=>l.go(r),params:I,meta:g,useHashNavigation:r=>l.mode(r?s.HASH:s.HISTORY),mode:{hash:()=>l.mode(s.HASH),history:()=>l.mode(s.HISTORY),memory:()=>l.mode(s.MEMORY)}}}function A(e){let r=h(e,"href"),a=h(e,"exact",!0),n=h(e,"active-class",!0,"active");return {destroy:m.subscribe(t=>{let o=b(r,t.path);o&&(o.exact&&a||!a)?e.classList.add(n):e.classList.remove(n);})}}function T(e){let r=a=>{let n=a.target.closest("a[href]"),t=n&&h(n,"target",!1,"_self"),o=n&&h(n,"tinro-ignore"),i=a.ctrlKey||a.metaKey||a.altKey||a.shiftKey;if(t=="_self"&&!o&&!i&&n){let c=n.getAttribute("href").replace(/^\/#/,"");/^\/\/|^[a-zA-Z]+:/.test(c)||(a.preventDefault(),e(c.startsWith("/")?c:n.href.replace(window.location.origin,"")));}};return addEventListener("click",r),()=>removeEventListener("click",r)}function I(){return getContext("tinro").meta.params}

    /* node_modules\tinro\cmp\Route.svelte generated by Svelte v3.32.3 */

    const get_default_slot_changes = dirty => ({
    	params: dirty & /*params*/ 2,
    	meta: dirty & /*meta*/ 4
    });

    const get_default_slot_context = ctx => ({
    	params: /*params*/ ctx[1],
    	meta: /*meta*/ ctx[2]
    });

    // (32:0) {#if showContent}
    function create_if_block(ctx) {
    	let current;
    	const default_slot_template = /*#slots*/ ctx[9].default;
    	const default_slot = create_slot(default_slot_template, ctx, /*$$scope*/ ctx[8], get_default_slot_context);

    	const block = {
    		c: function create() {
    			if (default_slot) default_slot.c();
    		},
    		m: function mount(target, anchor) {
    			if (default_slot) {
    				default_slot.m(target, anchor);
    			}

    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (default_slot) {
    				if (default_slot.p && dirty & /*$$scope, params, meta*/ 262) {
    					update_slot(default_slot, default_slot_template, ctx, /*$$scope*/ ctx[8], dirty, get_default_slot_changes, get_default_slot_context);
    				}
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(default_slot, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(default_slot, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (default_slot) default_slot.d(detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block.name,
    		type: "if",
    		source: "(32:0) {#if showContent}",
    		ctx
    	});

    	return block;
    }

    function create_fragment(ctx) {
    	let if_block_anchor;
    	let current;
    	let if_block = /*showContent*/ ctx[0] && create_if_block(ctx);

    	const block = {
    		c: function create() {
    			if (if_block) if_block.c();
    			if_block_anchor = empty();
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			if (if_block) if_block.m(target, anchor);
    			insert_dev(target, if_block_anchor, anchor);
    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*showContent*/ ctx[0]) {
    				if (if_block) {
    					if_block.p(ctx, dirty);

    					if (dirty & /*showContent*/ 1) {
    						transition_in(if_block, 1);
    					}
    				} else {
    					if_block = create_if_block(ctx);
    					if_block.c();
    					transition_in(if_block, 1);
    					if_block.m(if_block_anchor.parentNode, if_block_anchor);
    				}
    			} else if (if_block) {
    				group_outros();

    				transition_out(if_block, 1, 1, () => {
    					if_block = null;
    				});

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(if_block);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(if_block);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (if_block) if_block.d(detaching);
    			if (detaching) detach_dev(if_block_anchor);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("Route", slots, ['default']);
    	let { path = "/*" } = $$props;
    	let { fallback = false } = $$props;
    	let { redirect = false } = $$props;
    	let { firstmatch = false } = $$props;
    	let { breadcrumb = null } = $$props;
    	let showContent = false;
    	let params = {}; /* DEPRECATED */
    	let meta = {};

    	const route = $({
    		fallback,
    		onShow() {
    			$$invalidate(0, showContent = true);
    		},
    		onHide() {
    			$$invalidate(0, showContent = false);
    		},
    		onMeta(newmeta) {
    			$$invalidate(2, meta = newmeta);
    			$$invalidate(1, params = meta.params); /* DEPRECATED */
    		}
    	});

    	const writable_props = ["path", "fallback", "redirect", "firstmatch", "breadcrumb"];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console.warn(`<Route> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ("path" in $$props) $$invalidate(3, path = $$props.path);
    		if ("fallback" in $$props) $$invalidate(4, fallback = $$props.fallback);
    		if ("redirect" in $$props) $$invalidate(5, redirect = $$props.redirect);
    		if ("firstmatch" in $$props) $$invalidate(6, firstmatch = $$props.firstmatch);
    		if ("breadcrumb" in $$props) $$invalidate(7, breadcrumb = $$props.breadcrumb);
    		if ("$$scope" in $$props) $$invalidate(8, $$scope = $$props.$$scope);
    	};

    	$$self.$capture_state = () => ({
    		createRouteObject: $,
    		path,
    		fallback,
    		redirect,
    		firstmatch,
    		breadcrumb,
    		showContent,
    		params,
    		meta,
    		route
    	});

    	$$self.$inject_state = $$props => {
    		if ("path" in $$props) $$invalidate(3, path = $$props.path);
    		if ("fallback" in $$props) $$invalidate(4, fallback = $$props.fallback);
    		if ("redirect" in $$props) $$invalidate(5, redirect = $$props.redirect);
    		if ("firstmatch" in $$props) $$invalidate(6, firstmatch = $$props.firstmatch);
    		if ("breadcrumb" in $$props) $$invalidate(7, breadcrumb = $$props.breadcrumb);
    		if ("showContent" in $$props) $$invalidate(0, showContent = $$props.showContent);
    		if ("params" in $$props) $$invalidate(1, params = $$props.params);
    		if ("meta" in $$props) $$invalidate(2, meta = $$props.meta);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	$$self.$$.update = () => {
    		if ($$self.$$.dirty & /*path, redirect, firstmatch, breadcrumb*/ 232) {
    			route.update({ path, redirect, firstmatch, breadcrumb });
    		}
    	};

    	return [
    		showContent,
    		params,
    		meta,
    		path,
    		fallback,
    		redirect,
    		firstmatch,
    		breadcrumb,
    		$$scope,
    		slots
    	];
    }

    class Route extends SvelteComponentDev {
    	constructor(options) {
    		super(options);

    		init(this, options, instance, create_fragment, safe_not_equal, {
    			path: 3,
    			fallback: 4,
    			redirect: 5,
    			firstmatch: 6,
    			breadcrumb: 7
    		});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Route",
    			options,
    			id: create_fragment.name
    		});
    	}

    	get path() {
    		throw new Error("<Route>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set path(value) {
    		throw new Error("<Route>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get fallback() {
    		throw new Error("<Route>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set fallback(value) {
    		throw new Error("<Route>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get redirect() {
    		throw new Error("<Route>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set redirect(value) {
    		throw new Error("<Route>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get firstmatch() {
    		throw new Error("<Route>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set firstmatch(value) {
    		throw new Error("<Route>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get breadcrumb() {
    		throw new Error("<Route>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set breadcrumb(value) {
    		throw new Error("<Route>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    function cubicOut(t) {
        const f = t - 1.0;
        return f * f * f + 1.0;
    }

    function fade(node, { delay = 0, duration = 400, easing = identity } = {}) {
        const o = +getComputedStyle(node).opacity;
        return {
            delay,
            duration,
            easing,
            css: t => `opacity: ${t * o}`
        };
    }
    function fly(node, { delay = 0, duration = 400, easing = cubicOut, x = 0, y = 0, opacity = 0 } = {}) {
        const style = getComputedStyle(node);
        const target_opacity = +style.opacity;
        const transform = style.transform === 'none' ? '' : style.transform;
        const od = target_opacity * (1 - opacity);
        return {
            delay,
            duration,
            easing,
            css: (t, u) => `
			transform: ${transform} translate(${(1 - t) * x}px, ${(1 - t) * y}px);
			opacity: ${target_opacity - (od * u)}`
        };
    }

    function flip(node, animation, params = {}) {
        const style = getComputedStyle(node);
        const transform = style.transform === 'none' ? '' : style.transform;
        const scaleX = animation.from.width / node.clientWidth;
        const scaleY = animation.from.height / node.clientHeight;
        const dx = (animation.from.left - animation.to.left) / scaleX;
        const dy = (animation.from.top - animation.to.top) / scaleY;
        const d = Math.sqrt(dx * dx + dy * dy);
        const { delay = 0, duration = (d) => Math.sqrt(d) * 120, easing = cubicOut } = params;
        return {
            delay,
            duration: is_function(duration) ? duration(d) : duration,
            easing,
            css: (_t, u) => `transform: ${transform} translate(${u * dx}px, ${u * dy}px);`
        };
    }

    const createToast = () => {
      const { subscribe, update } = writable([]);
      let count = 0;
      let defaults = {};
      const push = (msg, opts = {}) => {
        const entry = { id: ++count, msg: msg, ...defaults, ...opts, theme: { ...defaults.theme, ...opts.theme } };
        update(n => entry.reversed ? [...n, entry] : [entry, ...n]);
        return count
      };
      const pop = id => {
        update(n => id ? n.filter(i => i.id !== id) : n.splice(1));
      };
      const set = (id, obj) => {
        update(n => {
          const idx = n.findIndex(i => i.id === id);
          if (idx > -1) {
            n[idx] = { ...n[idx], ...obj };
          }
          return n
        });
      };
      const _opts = (obj = {}) => {
        defaults = { ...defaults, ...obj, theme: { ...defaults.theme, ...obj.theme } };
        return defaults
      };
      return { subscribe, push, pop, set, _opts }
    };

    const toast = createToast();

    function is_date(obj) {
        return Object.prototype.toString.call(obj) === '[object Date]';
    }

    function get_interpolator(a, b) {
        if (a === b || a !== a)
            return () => a;
        const type = typeof a;
        if (type !== typeof b || Array.isArray(a) !== Array.isArray(b)) {
            throw new Error('Cannot interpolate values of different type');
        }
        if (Array.isArray(a)) {
            const arr = b.map((bi, i) => {
                return get_interpolator(a[i], bi);
            });
            return t => arr.map(fn => fn(t));
        }
        if (type === 'object') {
            if (!a || !b)
                throw new Error('Object cannot be null');
            if (is_date(a) && is_date(b)) {
                a = a.getTime();
                b = b.getTime();
                const delta = b - a;
                return t => new Date(a + t * delta);
            }
            const keys = Object.keys(b);
            const interpolators = {};
            keys.forEach(key => {
                interpolators[key] = get_interpolator(a[key], b[key]);
            });
            return t => {
                const result = {};
                keys.forEach(key => {
                    result[key] = interpolators[key](t);
                });
                return result;
            };
        }
        if (type === 'number') {
            const delta = b - a;
            return t => a + t * delta;
        }
        throw new Error(`Cannot interpolate ${type} values`);
    }
    function tweened(value, defaults = {}) {
        const store = writable(value);
        let task;
        let target_value = value;
        function set(new_value, opts) {
            if (value == null) {
                store.set(value = new_value);
                return Promise.resolve();
            }
            target_value = new_value;
            let previous_task = task;
            let started = false;
            let { delay = 0, duration = 400, easing = identity, interpolate = get_interpolator } = assign(assign({}, defaults), opts);
            if (duration === 0) {
                if (previous_task) {
                    previous_task.abort();
                    previous_task = null;
                }
                store.set(value = target_value);
                return Promise.resolve();
            }
            const start = now() + delay;
            let fn;
            task = loop(now => {
                if (now < start)
                    return true;
                if (!started) {
                    fn = interpolate(value, new_value);
                    if (typeof duration === 'function')
                        duration = duration(value, new_value);
                    started = true;
                }
                if (previous_task) {
                    previous_task.abort();
                    previous_task = null;
                }
                const elapsed = now - start;
                if (elapsed > duration) {
                    store.set(value = new_value);
                    return false;
                }
                // @ts-ignore
                store.set(value = fn(easing(elapsed / duration)));
                return true;
            });
            return task.promise;
        }
        return {
            set,
            update: (fn, opts) => set(fn(target_value, value), opts),
            subscribe: store.subscribe
        };
    }

    /* node_modules\@zerodevx\svelte-toast\src\ToastItem.svelte generated by Svelte v3.32.3 */
    const file = "node_modules\\@zerodevx\\svelte-toast\\src\\ToastItem.svelte";

    // (80:2) {#if item.dismissable}
    function create_if_block$1(ctx) {
    	let div;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			div = element("div");
    			div.textContent = "✕";
    			attr_dev(div, "class", "_toastBtn svelte-vfz6wa");
    			attr_dev(div, "role", "button");
    			attr_dev(div, "tabindex", "-1");
    			add_location(div, file, 80, 2, 1871);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);

    			if (!mounted) {
    				dispose = listen_dev(div, "click", /*click_handler*/ ctx[4], false, false, false);
    				mounted = true;
    			}
    		},
    		p: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$1.name,
    		type: "if",
    		source: "(80:2) {#if item.dismissable}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$1(ctx) {
    	let div1;
    	let div0;
    	let t0_value = /*item*/ ctx[0].msg + "";
    	let t0;
    	let t1;
    	let t2;
    	let progress_1;
    	let if_block = /*item*/ ctx[0].dismissable && create_if_block$1(ctx);

    	const block = {
    		c: function create() {
    			div1 = element("div");
    			div0 = element("div");
    			t0 = text(t0_value);
    			t1 = space();
    			if (if_block) if_block.c();
    			t2 = space();
    			progress_1 = element("progress");
    			attr_dev(div0, "class", "_toastMsg svelte-vfz6wa");
    			add_location(div0, file, 77, 2, 1803);
    			attr_dev(progress_1, "class", "_toastBar svelte-vfz6wa");
    			progress_1.value = /*$progress*/ ctx[1];
    			add_location(progress_1, file, 83, 2, 1977);
    			attr_dev(div1, "class", "_toastItem svelte-vfz6wa");
    			add_location(div1, file, 76, 0, 1776);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div1, anchor);
    			append_dev(div1, div0);
    			append_dev(div0, t0);
    			append_dev(div1, t1);
    			if (if_block) if_block.m(div1, null);
    			append_dev(div1, t2);
    			append_dev(div1, progress_1);
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*item*/ 1 && t0_value !== (t0_value = /*item*/ ctx[0].msg + "")) set_data_dev(t0, t0_value);

    			if (/*item*/ ctx[0].dismissable) {
    				if (if_block) {
    					if_block.p(ctx, dirty);
    				} else {
    					if_block = create_if_block$1(ctx);
    					if_block.c();
    					if_block.m(div1, t2);
    				}
    			} else if (if_block) {
    				if_block.d(1);
    				if_block = null;
    			}

    			if (dirty & /*$progress*/ 2) {
    				prop_dev(progress_1, "value", /*$progress*/ ctx[1]);
    			}
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div1);
    			if (if_block) if_block.d();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$1.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$1($$self, $$props, $$invalidate) {
    	let $progress;
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("ToastItem", slots, []);
    	let { item } = $$props;
    	const progress = tweened(item.initial, { duration: item.duration, easing: identity });
    	validate_store(progress, "progress");
    	component_subscribe($$self, progress, value => $$invalidate(1, $progress = value));
    	let prevProgress = item.initial;
    	const writable_props = ["item"];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console.warn(`<ToastItem> was created with unknown prop '${key}'`);
    	});

    	const click_handler = () => toast.pop(item.id);

    	$$self.$$set = $$props => {
    		if ("item" in $$props) $$invalidate(0, item = $$props.item);
    	};

    	$$self.$capture_state = () => ({
    		tweened,
    		linear: identity,
    		toast,
    		item,
    		progress,
    		prevProgress,
    		$progress
    	});

    	$$self.$inject_state = $$props => {
    		if ("item" in $$props) $$invalidate(0, item = $$props.item);
    		if ("prevProgress" in $$props) $$invalidate(3, prevProgress = $$props.prevProgress);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	$$self.$$.update = () => {
    		if ($$self.$$.dirty & /*prevProgress, item*/ 9) {
    			if (prevProgress !== item.progress) {
    				if (item.progress === 1 || item.progress === 0) {
    					progress.set(item.progress).then(() => toast.pop(item.id));
    				} else {
    					progress.set(item.progress);
    				}

    				$$invalidate(3, prevProgress = item.progress);
    			}
    		}
    	};

    	return [item, $progress, progress, prevProgress, click_handler];
    }

    class ToastItem extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$1, create_fragment$1, safe_not_equal, { item: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "ToastItem",
    			options,
    			id: create_fragment$1.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*item*/ ctx[0] === undefined && !("item" in props)) {
    			console.warn("<ToastItem> was created without expected prop 'item'");
    		}
    	}

    	get item() {
    		throw new Error("<ToastItem>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set item(value) {
    		throw new Error("<ToastItem>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* node_modules\@zerodevx\svelte-toast\src\SvelteToast.svelte generated by Svelte v3.32.3 */

    const { Object: Object_1 } = globals;
    const file$1 = "node_modules\\@zerodevx\\svelte-toast\\src\\SvelteToast.svelte";

    function get_each_context(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[4] = list[i];
    	return child_ctx;
    }

    // (39:2) {#each $toast as item (item.id)}
    function create_each_block(key_1, ctx) {
    	let li;
    	let toastitem;
    	let t;
    	let li_style_value;
    	let li_intro;
    	let li_outro;
    	let rect;
    	let stop_animation = noop;
    	let current;

    	toastitem = new ToastItem({
    			props: { item: /*item*/ ctx[4] },
    			$$inline: true
    		});

    	const block = {
    		key: key_1,
    		first: null,
    		c: function create() {
    			li = element("li");
    			create_component(toastitem.$$.fragment);
    			t = space();
    			attr_dev(li, "style", li_style_value = /*getCss*/ ctx[1](/*item*/ ctx[4].theme));
    			add_location(li, file$1, 39, 2, 830);
    			this.first = li;
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, li, anchor);
    			mount_component(toastitem, li, null);
    			append_dev(li, t);
    			current = true;
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    			const toastitem_changes = {};
    			if (dirty & /*$toast*/ 1) toastitem_changes.item = /*item*/ ctx[4];
    			toastitem.$set(toastitem_changes);

    			if (!current || dirty & /*$toast*/ 1 && li_style_value !== (li_style_value = /*getCss*/ ctx[1](/*item*/ ctx[4].theme))) {
    				attr_dev(li, "style", li_style_value);
    			}
    		},
    		r: function measure() {
    			rect = li.getBoundingClientRect();
    		},
    		f: function fix() {
    			fix_position(li);
    			stop_animation();
    			add_transform(li, rect);
    		},
    		a: function animate() {
    			stop_animation();
    			stop_animation = create_animation(li, rect, flip, { duration: 200 });
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(toastitem.$$.fragment, local);

    			add_render_callback(() => {
    				if (li_outro) li_outro.end(1);
    				if (!li_intro) li_intro = create_in_transition(li, fly, /*item*/ ctx[4].intro);
    				li_intro.start();
    			});

    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(toastitem.$$.fragment, local);
    			if (li_intro) li_intro.invalidate();
    			li_outro = create_out_transition(li, fade, {});
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(li);
    			destroy_component(toastitem);
    			if (detaching && li_outro) li_outro.end();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block.name,
    		type: "each",
    		source: "(39:2) {#each $toast as item (item.id)}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$2(ctx) {
    	let ul;
    	let each_blocks = [];
    	let each_1_lookup = new Map();
    	let current;
    	let each_value = /*$toast*/ ctx[0];
    	validate_each_argument(each_value);
    	const get_key = ctx => /*item*/ ctx[4].id;
    	validate_each_keys(ctx, each_value, get_each_context, get_key);

    	for (let i = 0; i < each_value.length; i += 1) {
    		let child_ctx = get_each_context(ctx, each_value, i);
    		let key = get_key(child_ctx);
    		each_1_lookup.set(key, each_blocks[i] = create_each_block(key, child_ctx));
    	}

    	const block = {
    		c: function create() {
    			ul = element("ul");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			attr_dev(ul, "class", "svelte-ivwmun");
    			add_location(ul, file$1, 37, 0, 788);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, ul, anchor);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(ul, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*getCss, $toast*/ 3) {
    				each_value = /*$toast*/ ctx[0];
    				validate_each_argument(each_value);
    				group_outros();
    				for (let i = 0; i < each_blocks.length; i += 1) each_blocks[i].r();
    				validate_each_keys(ctx, each_value, get_each_context, get_key);
    				each_blocks = update_keyed_each(each_blocks, dirty, get_key, 1, ctx, each_value, each_1_lookup, ul, fix_and_outro_and_destroy_block, create_each_block, null, get_each_context);
    				for (let i = 0; i < each_blocks.length; i += 1) each_blocks[i].a();
    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(ul);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].d();
    			}
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$2.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$2($$self, $$props, $$invalidate) {
    	let $toast;
    	validate_store(toast, "toast");
    	component_subscribe($$self, toast, $$value => $$invalidate(0, $toast = $$value));
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("SvelteToast", slots, []);
    	let { options = {} } = $$props;

    	const defaults = {
    		duration: 4000,
    		dismissable: true,
    		initial: 1,
    		progress: 0,
    		reversed: false,
    		intro: { x: 256 },
    		theme: {}
    	};

    	toast._opts(defaults);
    	const getCss = theme => Object.keys(theme).reduce((a, c) => `${a}${c}:${theme[c]};`, "");
    	const writable_props = ["options"];

    	Object_1.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console.warn(`<SvelteToast> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ("options" in $$props) $$invalidate(2, options = $$props.options);
    	};

    	$$self.$capture_state = () => ({
    		fade,
    		fly,
    		flip,
    		toast,
    		ToastItem,
    		options,
    		defaults,
    		getCss,
    		$toast
    	});

    	$$self.$inject_state = $$props => {
    		if ("options" in $$props) $$invalidate(2, options = $$props.options);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	$$self.$$.update = () => {
    		if ($$self.$$.dirty & /*options*/ 4) {
    			toast._opts(options);
    		}
    	};

    	return [$toast, getCss, options];
    }

    class SvelteToast extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$2, create_fragment$2, safe_not_equal, { options: 2 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "SvelteToast",
    			options,
    			id: create_fragment$2.name
    		});
    	}

    	get options() {
    		throw new Error("<SvelteToast>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set options(value) {
    		throw new Error("<SvelteToast>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\App.svelte generated by Svelte v3.32.3 */

    const { console: console_1 } = globals;

    const file$2 = "src\\App.svelte";

    // (168:2) <Route path="/">
    function create_default_slot_2(ctx) {
    	let div;
    	let h2;
    	let t1;
    	let button0;
    	let t3;
    	let button1;
    	let t5;
    	let p;
    	let t7;
    	let pre;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			div = element("div");
    			h2 = element("h2");
    			h2.textContent = `${setMain}`;
    			t1 = space();
    			button0 = element("button");
    			button0.textContent = "Get request";
    			t3 = space();
    			button1 = element("button");
    			button1.textContent = "Всплывающее окно";
    			t5 = space();
    			p = element("p");
    			p.textContent = "Result:";
    			t7 = space();
    			pre = element("pre");
    			pre.textContent = `${/*parseСonfigSetupJson*/ ctx[9]("name")}`;
    			add_location(h2, file$2, 169, 4, 3870);
    			attr_dev(button0, "type", "button");
    			add_location(button0, file$2, 170, 4, 3894);
    			attr_dev(button1, "type", "button");
    			add_location(button1, file$2, 173, 4, 3985);
    			add_location(p, file$2, 176, 4, 4070);
    			add_location(pre, file$2, 177, 4, 4090);
    			attr_dev(div, "class", "head svelte-1ob313s");
    			add_location(div, file$2, 168, 3, 3846);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			append_dev(div, h2);
    			append_dev(div, t1);
    			append_dev(div, button0);
    			append_dev(div, t3);
    			append_dev(div, button1);
    			append_dev(div, t5);
    			append_dev(div, p);
    			append_dev(div, t7);
    			append_dev(div, pre);

    			if (!mounted) {
    				dispose = [
    					listen_dev(button0, "click", /*getСonfigSetupJson*/ ctx[8], false, false, false),
    					listen_dev(button1, "click", /*upgrade*/ ctx[10], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_2.name,
    		type: "slot",
    		source: "(168:2) <Route path=\\\"/\\\">",
    		ctx
    	});

    	return block;
    }

    // (191:11)         
    function fallback_block_1(ctx) {
    	let form;
    	let div2;
    	let div0;
    	let label0;
    	let t1;
    	let div1;
    	let input0;
    	let t2;
    	let div5;
    	let div3;
    	let label1;
    	let t4;
    	let div4;
    	let input1;
    	let t5;
    	let div7;
    	let div6;
    	let button;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			form = element("form");
    			div2 = element("div");
    			div0 = element("div");
    			label0 = element("label");
    			label0.textContent = `${wifiSsidLable}`;
    			t1 = space();
    			div1 = element("div");
    			input0 = element("input");
    			t2 = space();
    			div5 = element("div");
    			div3 = element("div");
    			label1 = element("label");
    			label1.textContent = `${wifiPasswdLable}`;
    			t4 = space();
    			div4 = element("div");
    			input1 = element("input");
    			t5 = space();
    			div7 = element("div");
    			div6 = element("div");
    			button = element("button");
    			button.textContent = "Сохранить";
    			attr_dev(label0, "for", "ssid");
    			attr_dev(label0, "class", "svelte-1ob313s");
    			add_location(label0, file$2, 194, 9, 4400);
    			attr_dev(div0, "class", "left-column svelte-1ob313s");
    			add_location(div0, file$2, 193, 8, 4364);
    			attr_dev(input0, "type", "text");
    			input0.value = /*routerssid*/ ctx[0];
    			attr_dev(input0, "class", "svelte-1ob313s");
    			add_location(input0, file$2, 197, 9, 4504);
    			attr_dev(div1, "class", "right-column svelte-1ob313s");
    			add_location(div1, file$2, 196, 8, 4467);
    			attr_dev(div2, "class", "row svelte-1ob313s");
    			add_location(div2, file$2, 192, 7, 4337);
    			attr_dev(label1, "for", "passwd");
    			attr_dev(label1, "class", "svelte-1ob313s");
    			add_location(label1, file$2, 203, 9, 4649);
    			attr_dev(div3, "class", "left-column svelte-1ob313s");
    			add_location(div3, file$2, 202, 8, 4613);
    			attr_dev(input1, "type", "password");
    			input1.value = /*routerpass*/ ctx[1];
    			attr_dev(input1, "class", "svelte-1ob313s");
    			add_location(input1, file$2, 207, 9, 4768);
    			attr_dev(div4, "class", "right-column svelte-1ob313s");
    			add_location(div4, file$2, 206, 8, 4731);
    			attr_dev(div5, "class", "row svelte-1ob313s");
    			add_location(div5, file$2, 201, 7, 4586);
    			attr_dev(button, "type", "button");
    			add_location(button, file$2, 213, 9, 4919);
    			attr_dev(div6, "class", "center-column svelte-1ob313s");
    			add_location(div6, file$2, 212, 8, 4881);
    			attr_dev(div7, "class", "row svelte-1ob313s");
    			add_location(div7, file$2, 211, 7, 4854);
    			add_location(form, file$2, 191, 6, 4322);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, form, anchor);
    			append_dev(form, div2);
    			append_dev(div2, div0);
    			append_dev(div0, label0);
    			append_dev(div2, t1);
    			append_dev(div2, div1);
    			append_dev(div1, input0);
    			append_dev(form, t2);
    			append_dev(form, div5);
    			append_dev(div5, div3);
    			append_dev(div3, label1);
    			append_dev(div5, t4);
    			append_dev(div5, div4);
    			append_dev(div4, input1);
    			append_dev(form, t5);
    			append_dev(form, div7);
    			append_dev(div7, div6);
    			append_dev(div6, button);

    			if (!mounted) {
    				dispose = listen_dev(button, "click", /*getMqttData*/ ctx[7], false, false, false);
    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*routerssid*/ 1 && input0.value !== /*routerssid*/ ctx[0]) {
    				prop_dev(input0, "value", /*routerssid*/ ctx[0]);
    			}

    			if (dirty & /*routerpass*/ 2 && input1.value !== /*routerpass*/ ctx[1]) {
    				prop_dev(input1, "value", /*routerpass*/ ctx[1]);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(form);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: fallback_block_1.name,
    		type: "fallback",
    		source: "(191:11)         ",
    		ctx
    	});

    	return block;
    }

    // (184:2) <Route path="/wifi">
    function create_default_slot_1(ctx) {
    	let div0;
    	let h2;
    	let t1;
    	let div2;
    	let div1;
    	let current;
    	const default_slot_template = /*#slots*/ ctx[11].default;
    	const default_slot = create_slot(default_slot_template, ctx, /*$$scope*/ ctx[17], null);
    	const default_slot_or_fallback = default_slot || fallback_block_1(ctx);

    	const block = {
    		c: function create() {
    			div0 = element("div");
    			h2 = element("h2");
    			h2.textContent = `${setWifi}`;
    			t1 = space();
    			div2 = element("div");
    			div1 = element("div");
    			if (default_slot_or_fallback) default_slot_or_fallback.c();
    			add_location(h2, file$2, 185, 4, 4221);
    			attr_dev(div0, "class", "head svelte-1ob313s");
    			add_location(div0, file$2, 184, 3, 4197);
    			attr_dev(div1, "class", "box svelte-1ob313s");
    			add_location(div1, file$2, 189, 4, 4284);
    			attr_dev(div2, "class", "content svelte-1ob313s");
    			add_location(div2, file$2, 188, 3, 4257);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div0, anchor);
    			append_dev(div0, h2);
    			insert_dev(target, t1, anchor);
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div1);

    			if (default_slot_or_fallback) {
    				default_slot_or_fallback.m(div1, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (default_slot) {
    				if (default_slot.p && dirty & /*$$scope*/ 131072) {
    					update_slot(default_slot, default_slot_template, ctx, /*$$scope*/ ctx[17], dirty, null, null);
    				}
    			} else {
    				if (default_slot_or_fallback && default_slot_or_fallback.p && dirty & /*routerpass, routerssid*/ 3) {
    					default_slot_or_fallback.p(ctx, dirty);
    				}
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(default_slot_or_fallback, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(default_slot_or_fallback, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div0);
    			if (detaching) detach_dev(t1);
    			if (detaching) detach_dev(div2);
    			if (default_slot_or_fallback) default_slot_or_fallback.d(detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_1.name,
    		type: "slot",
    		source: "(184:2) <Route path=\\\"/wifi\\\">",
    		ctx
    	});

    	return block;
    }

    // (234:11)         
    function fallback_block(ctx) {
    	let form;
    	let div2;
    	let div0;
    	let label0;
    	let t1;
    	let div1;
    	let input0;
    	let t2;
    	let div5;
    	let div3;
    	let label1;
    	let t4;
    	let div4;
    	let input1;
    	let t5;
    	let div8;
    	let div6;
    	let label2;
    	let t7;
    	let div7;
    	let input2;
    	let t8;
    	let div11;
    	let div9;
    	let label3;
    	let t10;
    	let div10;
    	let input3;
    	let t11;
    	let div14;
    	let div12;
    	let label4;
    	let t13;
    	let div13;
    	let input4;
    	let t14;
    	let div16;
    	let div15;
    	let button;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			form = element("form");
    			div2 = element("div");
    			div0 = element("div");
    			label0 = element("label");
    			label0.textContent = `${mqttServerLable}`;
    			t1 = space();
    			div1 = element("div");
    			input0 = element("input");
    			t2 = space();
    			div5 = element("div");
    			div3 = element("div");
    			label1 = element("label");
    			label1.textContent = `${mqttPortLable}`;
    			t4 = space();
    			div4 = element("div");
    			input1 = element("input");
    			t5 = space();
    			div8 = element("div");
    			div6 = element("div");
    			label2 = element("label");
    			label2.textContent = `${mqttPrefixLable}`;
    			t7 = space();
    			div7 = element("div");
    			input2 = element("input");
    			t8 = space();
    			div11 = element("div");
    			div9 = element("div");
    			label3 = element("label");
    			label3.textContent = `${mqttUserLable}`;
    			t10 = space();
    			div10 = element("div");
    			input3 = element("input");
    			t11 = space();
    			div14 = element("div");
    			div12 = element("div");
    			label4 = element("label");
    			label4.textContent = `${mqttPasswdLable}`;
    			t13 = space();
    			div13 = element("div");
    			input4 = element("input");
    			t14 = space();
    			div16 = element("div");
    			div15 = element("div");
    			button = element("button");
    			button.textContent = "Сохранить";
    			attr_dev(label0, "for", "ssid");
    			attr_dev(label0, "class", "svelte-1ob313s");
    			add_location(label0, file$2, 237, 9, 5356);
    			attr_dev(div0, "class", "left-column svelte-1ob313s");
    			add_location(div0, file$2, 236, 8, 5320);
    			attr_dev(input0, "type", "text");
    			attr_dev(input0, "class", "svelte-1ob313s");
    			add_location(input0, file$2, 240, 9, 5462);
    			attr_dev(div1, "class", "right-column svelte-1ob313s");
    			add_location(div1, file$2, 239, 8, 5425);
    			attr_dev(div2, "class", "row svelte-1ob313s");
    			add_location(div2, file$2, 235, 7, 5293);
    			attr_dev(label1, "for", "passwd");
    			attr_dev(label1, "class", "svelte-1ob313s");
    			add_location(label1, file$2, 249, 9, 5644);
    			attr_dev(div3, "class", "left-column svelte-1ob313s");
    			add_location(div3, file$2, 248, 8, 5608);
    			attr_dev(input1, "type", "text");
    			attr_dev(input1, "class", "svelte-1ob313s");
    			add_location(input1, file$2, 252, 9, 5750);
    			attr_dev(div4, "class", "right-column svelte-1ob313s");
    			add_location(div4, file$2, 251, 8, 5713);
    			attr_dev(div5, "class", "row svelte-1ob313s");
    			add_location(div5, file$2, 247, 7, 5581);
    			attr_dev(label2, "for", "passwd");
    			attr_dev(label2, "class", "svelte-1ob313s");
    			add_location(label2, file$2, 258, 9, 5898);
    			attr_dev(div6, "class", "left-column svelte-1ob313s");
    			add_location(div6, file$2, 257, 8, 5862);
    			attr_dev(input2, "type", "text");
    			attr_dev(input2, "class", "svelte-1ob313s");
    			add_location(input2, file$2, 262, 9, 6017);
    			attr_dev(div7, "class", "right-column svelte-1ob313s");
    			add_location(div7, file$2, 261, 8, 5980);
    			attr_dev(div8, "class", "row svelte-1ob313s");
    			add_location(div8, file$2, 256, 7, 5835);
    			attr_dev(label3, "for", "passwd");
    			attr_dev(label3, "class", "svelte-1ob313s");
    			add_location(label3, file$2, 271, 9, 6199);
    			attr_dev(div9, "class", "left-column svelte-1ob313s");
    			add_location(div9, file$2, 270, 8, 6163);
    			attr_dev(input3, "type", "text");
    			attr_dev(input3, "class", "svelte-1ob313s");
    			add_location(input3, file$2, 274, 9, 6305);
    			attr_dev(div10, "class", "right-column svelte-1ob313s");
    			add_location(div10, file$2, 273, 8, 6268);
    			attr_dev(div11, "class", "row svelte-1ob313s");
    			add_location(div11, file$2, 269, 7, 6136);
    			attr_dev(label4, "for", "passwd");
    			attr_dev(label4, "class", "svelte-1ob313s");
    			add_location(label4, file$2, 280, 9, 6453);
    			attr_dev(div12, "class", "left-column svelte-1ob313s");
    			add_location(div12, file$2, 279, 8, 6417);
    			attr_dev(input4, "type", "password");
    			attr_dev(input4, "class", "svelte-1ob313s");
    			add_location(input4, file$2, 284, 9, 6572);
    			attr_dev(div13, "class", "right-column svelte-1ob313s");
    			add_location(div13, file$2, 283, 8, 6535);
    			attr_dev(div14, "class", "row svelte-1ob313s");
    			add_location(div14, file$2, 278, 7, 6390);
    			attr_dev(button, "type", "button");
    			add_location(button, file$2, 293, 9, 6758);
    			attr_dev(div15, "class", "center-column svelte-1ob313s");
    			add_location(div15, file$2, 292, 8, 6720);
    			attr_dev(div16, "class", "row svelte-1ob313s");
    			add_location(div16, file$2, 291, 7, 6693);
    			add_location(form, file$2, 234, 6, 5278);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, form, anchor);
    			append_dev(form, div2);
    			append_dev(div2, div0);
    			append_dev(div0, label0);
    			append_dev(div2, t1);
    			append_dev(div2, div1);
    			append_dev(div1, input0);
    			set_input_value(input0, /*mqttServer*/ ctx[2]);
    			append_dev(form, t2);
    			append_dev(form, div5);
    			append_dev(div5, div3);
    			append_dev(div3, label1);
    			append_dev(div5, t4);
    			append_dev(div5, div4);
    			append_dev(div4, input1);
    			set_input_value(input1, /*mqttPort*/ ctx[3]);
    			append_dev(form, t5);
    			append_dev(form, div8);
    			append_dev(div8, div6);
    			append_dev(div6, label2);
    			append_dev(div8, t7);
    			append_dev(div8, div7);
    			append_dev(div7, input2);
    			set_input_value(input2, /*mqttPrefix*/ ctx[4]);
    			append_dev(form, t8);
    			append_dev(form, div11);
    			append_dev(div11, div9);
    			append_dev(div9, label3);
    			append_dev(div11, t10);
    			append_dev(div11, div10);
    			append_dev(div10, input3);
    			set_input_value(input3, /*mqttUser*/ ctx[5]);
    			append_dev(form, t11);
    			append_dev(form, div14);
    			append_dev(div14, div12);
    			append_dev(div12, label4);
    			append_dev(div14, t13);
    			append_dev(div14, div13);
    			append_dev(div13, input4);
    			set_input_value(input4, /*mqttPass*/ ctx[6]);
    			append_dev(form, t14);
    			append_dev(form, div16);
    			append_dev(div16, div15);
    			append_dev(div15, button);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input0, "input", /*input0_input_handler*/ ctx[12]),
    					listen_dev(input1, "input", /*input1_input_handler*/ ctx[13]),
    					listen_dev(input2, "input", /*input2_input_handler*/ ctx[14]),
    					listen_dev(input3, "input", /*input3_input_handler*/ ctx[15]),
    					listen_dev(input4, "input", /*input4_input_handler*/ ctx[16]),
    					listen_dev(button, "click", /*getMqttData*/ ctx[7], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*mqttServer*/ 4 && input0.value !== /*mqttServer*/ ctx[2]) {
    				set_input_value(input0, /*mqttServer*/ ctx[2]);
    			}

    			if (dirty & /*mqttPort*/ 8 && input1.value !== /*mqttPort*/ ctx[3]) {
    				set_input_value(input1, /*mqttPort*/ ctx[3]);
    			}

    			if (dirty & /*mqttPrefix*/ 16 && input2.value !== /*mqttPrefix*/ ctx[4]) {
    				set_input_value(input2, /*mqttPrefix*/ ctx[4]);
    			}

    			if (dirty & /*mqttUser*/ 32 && input3.value !== /*mqttUser*/ ctx[5]) {
    				set_input_value(input3, /*mqttUser*/ ctx[5]);
    			}

    			if (dirty & /*mqttPass*/ 64 && input4.value !== /*mqttPass*/ ctx[6]) {
    				set_input_value(input4, /*mqttPass*/ ctx[6]);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(form);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: fallback_block.name,
    		type: "fallback",
    		source: "(234:11)         ",
    		ctx
    	});

    	return block;
    }

    // (227:2) <Route path="/mqtt">
    function create_default_slot(ctx) {
    	let div0;
    	let h2;
    	let t1;
    	let div2;
    	let div1;
    	let current;
    	const default_slot_template = /*#slots*/ ctx[11].default;
    	const default_slot = create_slot(default_slot_template, ctx, /*$$scope*/ ctx[17], null);
    	const default_slot_or_fallback = default_slot || fallback_block(ctx);

    	const block = {
    		c: function create() {
    			div0 = element("div");
    			h2 = element("h2");
    			h2.textContent = `${setMqtt}`;
    			t1 = space();
    			div2 = element("div");
    			div1 = element("div");
    			if (default_slot_or_fallback) default_slot_or_fallback.c();
    			add_location(h2, file$2, 228, 4, 5177);
    			attr_dev(div0, "class", "head svelte-1ob313s");
    			add_location(div0, file$2, 227, 3, 5153);
    			attr_dev(div1, "class", "box svelte-1ob313s");
    			add_location(div1, file$2, 232, 4, 5240);
    			attr_dev(div2, "class", "content svelte-1ob313s");
    			add_location(div2, file$2, 231, 3, 5213);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div0, anchor);
    			append_dev(div0, h2);
    			insert_dev(target, t1, anchor);
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div1);

    			if (default_slot_or_fallback) {
    				default_slot_or_fallback.m(div1, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (default_slot) {
    				if (default_slot.p && dirty & /*$$scope*/ 131072) {
    					update_slot(default_slot, default_slot_template, ctx, /*$$scope*/ ctx[17], dirty, null, null);
    				}
    			} else {
    				if (default_slot_or_fallback && default_slot_or_fallback.p && dirty & /*mqttPass, mqttUser, mqttPrefix, mqttPort, mqttServer*/ 124) {
    					default_slot_or_fallback.p(ctx, dirty);
    				}
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(default_slot_or_fallback, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(default_slot_or_fallback, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div0);
    			if (detaching) detach_dev(t1);
    			if (detaching) detach_dev(div2);
    			if (default_slot_or_fallback) default_slot_or_fallback.d(detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot.name,
    		type: "slot",
    		source: "(227:2) <Route path=\\\"/mqtt\\\">",
    		ctx
    	});

    	return block;
    }

    function create_fragment$3(ctx) {
    	let div;
    	let input;
    	let t0;
    	let label;
    	let span;
    	let t1;
    	let ul0;
    	let li0;
    	let a0;
    	let t3;
    	let li1;
    	let a1;
    	let t5;
    	let li2;
    	let a2;
    	let t7;
    	let ul1;
    	let route0;
    	let t8;
    	let route1;
    	let t9;
    	let route2;
    	let current;
    	let mounted;
    	let dispose;

    	route0 = new Route({
    			props: {
    				path: "/",
    				$$slots: { default: [create_default_slot_2] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	route1 = new Route({
    			props: {
    				path: "/wifi",
    				$$slots: { default: [create_default_slot_1] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	route2 = new Route({
    			props: {
    				path: "/mqtt",
    				$$slots: { default: [create_default_slot] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			div = element("div");
    			input = element("input");
    			t0 = space();
    			label = element("label");
    			span = element("span");
    			t1 = space();
    			ul0 = element("ul");
    			li0 = element("li");
    			a0 = element("a");
    			a0.textContent = `${setMain}`;
    			t3 = space();
    			li1 = element("li");
    			a1 = element("a");
    			a1.textContent = `${setWifi}`;
    			t5 = space();
    			li2 = element("li");
    			a2 = element("a");
    			a2.textContent = `${setMqtt}`;
    			t7 = space();
    			ul1 = element("ul");
    			create_component(route0.$$.fragment);
    			t8 = space();
    			create_component(route1.$$.fragment);
    			t9 = space();
    			create_component(route2.$$.fragment);
    			attr_dev(input, "id", "menu__toggle");
    			attr_dev(input, "type", "checkbox");
    			attr_dev(input, "class", "svelte-1ob313s");
    			add_location(input, file$2, 142, 1, 3314);
    			attr_dev(span, "class", "svelte-1ob313s");
    			add_location(span, file$2, 144, 2, 3408);
    			attr_dev(label, "class", "menu__btn svelte-1ob313s");
    			attr_dev(label, "for", "menu__toggle");
    			add_location(label, file$2, 143, 1, 3360);
    			attr_dev(a0, "class", "menu__item svelte-1ob313s");
    			attr_dev(a0, "href", "/");
    			add_location(a0, file$2, 148, 3, 3465);
    			add_location(li0, file$2, 147, 2, 3456);
    			attr_dev(a1, "class", "menu__item svelte-1ob313s");
    			attr_dev(a1, "href", "/wifi");
    			add_location(a1, file$2, 154, 3, 3574);
    			add_location(li1, file$2, 153, 2, 3565);
    			attr_dev(a2, "class", "menu__item svelte-1ob313s");
    			attr_dev(a2, "href", "/mqtt");
    			add_location(a2, file$2, 160, 3, 3687);
    			add_location(li2, file$2, 159, 2, 3678);
    			attr_dev(ul0, "class", "menu__box svelte-1ob313s");
    			add_location(ul0, file$2, 146, 1, 3430);
    			attr_dev(ul1, "class", "menu__main svelte-1ob313s");
    			add_location(ul1, file$2, 166, 1, 3798);
    			attr_dev(div, "class", "hamburger-menu");
    			add_location(div, file$2, 141, 0, 3283);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			append_dev(div, input);
    			append_dev(div, t0);
    			append_dev(div, label);
    			append_dev(label, span);
    			append_dev(div, t1);
    			append_dev(div, ul0);
    			append_dev(ul0, li0);
    			append_dev(li0, a0);
    			append_dev(ul0, t3);
    			append_dev(ul0, li1);
    			append_dev(li1, a1);
    			append_dev(ul0, t5);
    			append_dev(ul0, li2);
    			append_dev(li2, a2);
    			append_dev(div, t7);
    			append_dev(div, ul1);
    			mount_component(route0, ul1, null);
    			append_dev(ul1, t8);
    			mount_component(route1, ul1, null);
    			append_dev(ul1, t9);
    			mount_component(route2, ul1, null);
    			current = true;

    			if (!mounted) {
    				dispose = [
    					listen_dev(a0, "click", /*getСonfigSetupJson*/ ctx[8], false, false, false),
    					listen_dev(a1, "click", /*getСonfigSetupJson*/ ctx[8], false, false, false),
    					listen_dev(a2, "click", /*getСonfigSetupJson*/ ctx[8], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			const route0_changes = {};

    			if (dirty & /*$$scope*/ 131072) {
    				route0_changes.$$scope = { dirty, ctx };
    			}

    			route0.$set(route0_changes);
    			const route1_changes = {};

    			if (dirty & /*$$scope, routerpass, routerssid*/ 131075) {
    				route1_changes.$$scope = { dirty, ctx };
    			}

    			route1.$set(route1_changes);
    			const route2_changes = {};

    			if (dirty & /*$$scope, mqttPass, mqttUser, mqttPrefix, mqttPort, mqttServer*/ 131196) {
    				route2_changes.$$scope = { dirty, ctx };
    			}

    			route2.$set(route2_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(route0.$$.fragment, local);
    			transition_in(route1.$$.fragment, local);
    			transition_in(route2.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(route0.$$.fragment, local);
    			transition_out(route1.$$.fragment, local);
    			transition_out(route2.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			destroy_component(route0);
    			destroy_component(route1);
    			destroy_component(route2);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$3.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    const setMain = "Устройство";
    const setWifi = "WiFi";
    const setMqtt = "MQTT";
    const wifiSsidLable = "Название сети:";
    const wifiPasswdLable = "Пароль:";
    const mqttServerLable = "Имя сервера:";
    const mqttPortLable = "Номер порта:";
    const mqttPrefixLable = "Префикс:";
    const mqttUserLable = "Имя пользователя:";
    const mqttPasswdLable = "Пароль:";

    function instance$3($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("App", slots, ['default']);
    	m.mode.hash(); // enables hash navigation method

    	const app = new SvelteToast({
    			target: document.body,
    			props: { options: {} }
    		});

    	//секция переменных============================================
    	let myip = document.location.hostname;

    	let configSetupJson = "{}";
    	let routerssid;
    	let routerpass;
    	let mqttServer;
    	let mqttPort;
    	let mqttPrefix;
    	let mqttUser;
    	let mqttPass;

    	//функции=======================================================
    	onMount(async () => {
    		getСonfigSetupJson();
    	});

    	async function getMqttData() {
    		let res = await fetch("http://" + myip + "/set?mqttServer=" + mqttServer + "&mqttPort=" + mqttPort + "&mqttPrefix=" + mqttPrefix + "&mqttUser=" + mqttUser + "&mqttPass=" + mqttPass, { mode: "no-cors", method: "GET" });

    		if (res.ok) {
    			pushGreen("Saved!");
    		} //pushRed();
    	} //if (res.ok) {
    	//	let text = await res.text();

    	//	alert("received msg: " + text);
    	//} else {
    	//	alert("status " + res.status);
    	//}
    	async function getСonfigSetupJson() {
    		let res = await fetch("http://" + myip + "/config.setup.json", { mode: "no-cors", method: "GET" });

    		if (res.ok) {
    			configSetupJson = await res.json();
    		} else {
    			//alert("status " + res.status);
    			console.log("error", res.status);
    		}

    		getValues();
    	}

    	function parseСonfigSetupJson(key) {
    		let result = configSetupJson[key];
    		return result;
    	}

    	function getValues() {
    		$$invalidate(0, routerssid = parseСonfigSetupJson("routerssid"));
    		$$invalidate(1, routerpass = parseСonfigSetupJson("routerpass"));
    		$$invalidate(2, mqttServer = parseСonfigSetupJson("mqttServer"));
    		$$invalidate(3, mqttPort = parseСonfigSetupJson("mqttPort"));
    		$$invalidate(4, mqttPrefix = parseСonfigSetupJson("mqttPrefix"));
    		$$invalidate(5, mqttUser = parseСonfigSetupJson("mqttUser"));
    		$$invalidate(6, mqttPass = parseСonfigSetupJson("mqttPass"));
    	}

    	function pushGreen(text) {
    		toast.push(text, {
    			duration: 500,
    			theme: {
    				"--toastBackground": "#48BB78",
    				"--toastProgressBackground": "#2F855A"
    			}
    		});
    	}

    	function pushRed() {
    		toast.push("Error!", {
    			theme: {
    				"--toastBackground": "#F56565",
    				"--toastProgressBackground": "#C53030"
    			}
    		});
    	}

    	function upgrade() {
    		toast.push("Upgrade in progress...", {
    			duration: 20000,
    			theme: {
    				"--toastBackground": "#48BB78",
    				"--toastProgressBackground": "#2F855A"
    			}
    		});
    	}

    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console_1.warn(`<App> was created with unknown prop '${key}'`);
    	});

    	function input0_input_handler() {
    		mqttServer = this.value;
    		$$invalidate(2, mqttServer);
    	}

    	function input1_input_handler() {
    		mqttPort = this.value;
    		$$invalidate(3, mqttPort);
    	}

    	function input2_input_handler() {
    		mqttPrefix = this.value;
    		$$invalidate(4, mqttPrefix);
    	}

    	function input3_input_handler() {
    		mqttUser = this.value;
    		$$invalidate(5, mqttUser);
    	}

    	function input4_input_handler() {
    		mqttPass = this.value;
    		$$invalidate(6, mqttPass);
    	}

    	$$self.$$set = $$props => {
    		if ("$$scope" in $$props) $$invalidate(17, $$scope = $$props.$$scope);
    	};

    	$$self.$capture_state = () => ({
    		Route,
    		router: m,
    		active: A,
    		SvelteToast,
    		toast,
    		app,
    		onMount,
    		myip,
    		configSetupJson,
    		setMain,
    		setWifi,
    		setMqtt,
    		wifiSsidLable,
    		wifiPasswdLable,
    		mqttServerLable,
    		mqttPortLable,
    		mqttPrefixLable,
    		mqttUserLable,
    		mqttPasswdLable,
    		routerssid,
    		routerpass,
    		mqttServer,
    		mqttPort,
    		mqttPrefix,
    		mqttUser,
    		mqttPass,
    		getMqttData,
    		getСonfigSetupJson,
    		parseСonfigSetupJson,
    		getValues,
    		pushGreen,
    		pushRed,
    		upgrade
    	});

    	$$self.$inject_state = $$props => {
    		if ("myip" in $$props) myip = $$props.myip;
    		if ("configSetupJson" in $$props) configSetupJson = $$props.configSetupJson;
    		if ("routerssid" in $$props) $$invalidate(0, routerssid = $$props.routerssid);
    		if ("routerpass" in $$props) $$invalidate(1, routerpass = $$props.routerpass);
    		if ("mqttServer" in $$props) $$invalidate(2, mqttServer = $$props.mqttServer);
    		if ("mqttPort" in $$props) $$invalidate(3, mqttPort = $$props.mqttPort);
    		if ("mqttPrefix" in $$props) $$invalidate(4, mqttPrefix = $$props.mqttPrefix);
    		if ("mqttUser" in $$props) $$invalidate(5, mqttUser = $$props.mqttUser);
    		if ("mqttPass" in $$props) $$invalidate(6, mqttPass = $$props.mqttPass);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		routerssid,
    		routerpass,
    		mqttServer,
    		mqttPort,
    		mqttPrefix,
    		mqttUser,
    		mqttPass,
    		getMqttData,
    		getСonfigSetupJson,
    		parseСonfigSetupJson,
    		upgrade,
    		slots,
    		input0_input_handler,
    		input1_input_handler,
    		input2_input_handler,
    		input3_input_handler,
    		input4_input_handler,
    		$$scope
    	];
    }

    class App extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$3, create_fragment$3, safe_not_equal, {});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "App",
    			options,
    			id: create_fragment$3.name
    		});
    	}
    }

    const app = new App({
    	target: document.body,
    	props: {
    		name: 'world'
    	}
    });

    return app;

}());
//# sourceMappingURL=bundle.js.map
