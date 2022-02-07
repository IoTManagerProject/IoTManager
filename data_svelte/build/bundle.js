
(function(l, r) { if (!l || l.getElementById('livereloadscript')) return; r = l.createElement('script'); r.async = 1; r.src = '//' + (self.location.host || 'localhost').split(':')[0] + ':35729/livereload.js?snipver=1'; r.id = 'livereloadscript'; l.getElementsByTagName('head')[0].appendChild(r) })(self.document);
var app = (function () {
    'use strict';

    function noop() { }
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
    function update_slot_base(slot, slot_definition, ctx, $$scope, slot_changes, get_slot_context_fn) {
        if (slot_changes) {
            const slot_context = get_slot_context(slot_definition, ctx, $$scope, get_slot_context_fn);
            slot.p(slot_context, slot_changes);
        }
    }
    function get_all_dirty_from_scope($$scope) {
        if ($$scope.ctx.length > 32) {
            const dirty = [];
            const length = $$scope.ctx.length / 32;
            for (let i = 0; i < length; i++) {
                dirty[i] = -1;
            }
            return dirty;
        }
        return -1;
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
    function destroy_each(iterations, detaching) {
        for (let i = 0; i < iterations.length; i += 1) {
            if (iterations[i])
                iterations[i].d(detaching);
        }
    }
    function element(name) {
        return document.createElement(name);
    }
    function svg_element(name) {
        return document.createElementNS('http://www.w3.org/2000/svg', name);
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
    function to_number(value) {
        return value === '' ? null : +value;
    }
    function children(element) {
        return Array.from(element.childNodes);
    }
    function set_input_value(input, value) {
        input.value = value == null ? '' : value;
    }
    function select_option(select, value) {
        for (let i = 0; i < select.options.length; i += 1) {
            const option = select.options[i];
            if (option.__value === value) {
                option.selected = true;
                return;
            }
        }
        select.selectedIndex = -1; // no option should be selected
    }
    function select_value(select) {
        const selected_option = select.querySelector(':checked') || select.options[0];
        return selected_option && selected_option.__value;
    }
    function custom_event(type, detail, bubbles = false) {
        const e = document.createEvent('CustomEvent');
        e.initCustomEvent(type, bubbles, false, detail);
        return e;
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
    function onDestroy(fn) {
        get_current_component().$$.on_destroy.push(fn);
    }
    function setContext(key, context) {
        get_current_component().$$.context.set(key, context);
    }
    function getContext(key) {
        return get_current_component().$$.context.get(key);
    }
    // TODO figure out if we still want to support
    // shorthand events, or if we want to implement
    // a real bubbling mechanism
    function bubble(component, event) {
        const callbacks = component.$$.callbacks[event.type];
        if (callbacks) {
            // @ts-ignore
            callbacks.slice().forEach(fn => fn.call(this, event));
        }
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
    function add_flush_callback(fn) {
        flush_callbacks.push(fn);
    }
    // flush() calls callbacks in this order:
    // 1. All beforeUpdate callbacks, in order: parents before children
    // 2. All bind:this callbacks, in reverse order: children before parents.
    // 3. All afterUpdate callbacks, in order: parents before children. EXCEPT
    //    for afterUpdates called during the initial onMount, which are called in
    //    reverse order: children before parents.
    // Since callbacks might update component values, which could trigger another
    // call to flush(), the following steps guard against this:
    // 1. During beforeUpdate, any updated components will be added to the
    //    dirty_components array and will cause a reentrant call to flush(). Because
    //    the flush index is kept outside the function, the reentrant call will pick
    //    up where the earlier call left off and go through all dirty components. The
    //    current_component value is saved and restored so that the reentrant call will
    //    not interfere with the "parent" flush() call.
    // 2. bind:this callbacks cannot trigger new flush() calls.
    // 3. During afterUpdate, any updated components will NOT have their afterUpdate
    //    callback called a second time; the seen_callbacks set, outside the flush()
    //    function, guarantees this behavior.
    const seen_callbacks = new Set();
    let flushidx = 0; // Do *not* move this inside the flush() function
    function flush() {
        const saved_component = current_component;
        do {
            // first, call beforeUpdate functions
            // and update components
            while (flushidx < dirty_components.length) {
                const component = dirty_components[flushidx];
                flushidx++;
                set_current_component(component);
                update(component.$$);
            }
            set_current_component(null);
            dirty_components.length = 0;
            flushidx = 0;
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
        seen_callbacks.clear();
        set_current_component(saved_component);
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

    const globals = (typeof window !== 'undefined'
        ? window
        : typeof globalThis !== 'undefined'
            ? globalThis
            : global);

    function bind(component, name, callback) {
        const index = component.$$.props[name];
        if (index !== undefined) {
            component.$$.bound[index] = callback;
            callback(component.$$.ctx[index]);
        }
    }
    function create_component(block) {
        block && block.c();
    }
    function mount_component(component, target, anchor, customElement) {
        const { fragment, on_mount, on_destroy, after_update } = component.$$;
        fragment && fragment.m(target, anchor);
        if (!customElement) {
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
        }
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
    function init(component, options, instance, create_fragment, not_equal, props, append_styles, dirty = [-1]) {
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
            on_disconnect: [],
            before_update: [],
            after_update: [],
            context: new Map(options.context || (parent_component ? parent_component.$$.context : [])),
            // everything else
            callbacks: blank_object(),
            dirty,
            skip_bound: false,
            root: options.target || parent_component.$$.root
        };
        append_styles && append_styles($$.root);
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
            mount_component(component, options.target, options.anchor, options.customElement);
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
        document.dispatchEvent(custom_event(type, Object.assign({ version: '3.46.4' }, detail), true));
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
        const subscribers = new Set();
        function set(new_value) {
            if (safe_not_equal(value, new_value)) {
                value = new_value;
                if (stop) { // store is ready
                    const run_queue = !subscriber_queue.length;
                    for (const subscriber of subscribers) {
                        subscriber[1]();
                        subscriber_queue.push(subscriber, value);
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
            subscribers.add(subscriber);
            if (subscribers.size === 1) {
                stop = start(set) || noop;
            }
            run(value);
            return () => {
                subscribers.delete(subscriber);
                if (subscribers.size === 0) {
                    stop();
                    stop = null;
                }
            };
        }
        return { set, update, subscribe };
    }

    function d(e,n=!1){return e=e.slice(e.startsWith("/#")?2:0,e.endsWith("/*")?-2:void 0),e.startsWith("/")||(e="/"+e),e==="/"&&(e=""),n&&!e.endsWith("/")&&(e+="/"),e}function b(e,n){e=d(e,!0),n=d(n,!0);let a=[],r={},t=!0,o=e.split("/").map(i=>i.startsWith(":")?(a.push(i.slice(1)),"([^\\/]+)"):i).join("\\/"),c=n.match(new RegExp(`^${o}$`));return c||(t=!1,c=n.match(new RegExp(`^${o}`))),c?(a.forEach((i,w)=>r[i]=c[w+1]),{exact:t,params:r,part:c[0].slice(0,-1)}):null}function x(e,n,a){if(a==="")return e;if(a[0]==="/")return a;let r=c=>c.split("/").filter(i=>i!==""),t=r(e),o=n?r(n):[];return "/"+o.map((c,i)=>t[i]).join("/")+"/"+a}function h(e,n,a,r){let t=[n,"data-"+n].reduce((o,c)=>{let i=e.getAttribute(c);return a&&e.removeAttribute(c),i===null?o:i},!1);return !r&&t===""?!0:t||r||!1}function v(e){let n=e.split("&").map(a=>a.split("=")).reduce((a,r)=>{let t=r[0];if(!t)return a;let o=r.length>1?r[r.length-1]:!0;return typeof o=="string"&&o.includes(",")&&(o=o.split(",")),a[t]===void 0?a[t]=[o]:a[t].push(o),a},{});return Object.entries(n).reduce((a,r)=>(a[r[0]]=r[1].length>1?r[1]:r[1][0],a),{})}function S(e){throw new Error("[Tinro] "+e)}var k=1,y=2,M=3,j=4;function C(e,n,a,r){return e===k?n&&n():e===y?a&&a():r&&r()}function W(){return !window||window.location.pathname==="srcdoc"?M:k}var s={HISTORY:k,HASH:y,MEMORY:M,OFF:j,run:C,getDeafault:W};var O,R,l=z();function z(){let e=s.getDeafault(),n,a=o=>window.onhashchange=window.onpopstate=R=null,r=o=>n&&n(_(e));function t(o){o&&(e=o),a(),e!==s.OFF&&s.run(e,c=>window.onpopstate=r,c=>window.onhashchange=r)&&r();}return {mode:o=>t(o),get:o=>_(e),go(o,c){D(e,o,c),r();},start(o){n=o,t();},stop(){n=null,t(s.OFF);}}}function D(e,n,a){let r=t=>history[a?"replaceState":"pushState"]({},"",t);s.run(e,t=>r(n),t=>r(`#${n}`),t=>R=n);}function _(e){let n=O,a=O=s.run(e,t=>window.location.pathname+window.location.search,t=>String(window.location.hash.slice(1)||"/"),t=>R||"/"),r=a.match(/^([^?#]+)(?:\?([^#]+))?(?:\#(.+))?$/);return {url:a,from:n,path:r[1]||"",query:v(r[2]||""),hash:r[3]||""}}function $(e){let n=getContext("tinro");n&&(n.exact||n.fallback)&&S(`${e.fallback?"<Route fallback>":`<Route path="${e.path}">`}  can't be inside ${n.fallback?"<Route fallback>":`<Route path="${n.path||"/"}"> with exact path`}`);let a=e.fallback?"fallbacks":"childs",r=writable({}),t={router:{},exact:!1,pattern:null,meta:{},parent:n,fallback:e.fallback,redirect:!1,firstmatch:!1,breadcrumb:null,matched:!1,childs:new Set,activeChilds:new Set,fallbacks:new Set,update(o){t.exact=!o.path.endsWith("/*"),t.pattern=d(`${t.parent&&t.parent.pattern||""}${o.path}`),t.redirect=o.redirect,t.firstmatch=o.firstmatch,t.breadcrumb=o.breadcrumb,t.match();},register:()=>{if(!!t.parent)return t.parent[a].add(t),()=>{t.parent[a].delete(t),t.router.un&&t.router.un();}},show:()=>{e.onShow(),!t.fallback&&t.parent&&t.parent.activeChilds.add(t);},hide:()=>{e.onHide(),!t.fallback&&t.parent&&t.parent.activeChilds.delete(t);},match:async()=>{t.matched=!1;let{path:o,url:c,from:i,query:w}=t.router,u=b(t.pattern,o);if(!t.fallback&&u&&t.redirect&&(!t.exact||t.exact&&u.exact)){await tick();let m=x(o,t.parent&&t.parent.pattern,t.redirect);return f.goto(m,!0)}if(t.meta=u&&{from:i,url:c,query:w,match:u.part,pattern:t.pattern,breadcrumbs:t.parent&&t.parent.meta&&t.parent.meta.breadcrumbs.slice()||[],params:u.params,subscribe:r.subscribe},t.breadcrumb&&t.meta&&t.meta.breadcrumbs.push({name:t.breadcrumb,path:u.part}),r.set(t.meta),u&&!t.fallback&&(!t.exact||t.exact&&u.exact)&&(!t.parent||!t.parent.firstmatch||!t.parent.matched)?(e.onMeta(t.meta),t.parent&&(t.parent.matched=!0),t.show()):t.hide(),await tick(),u&&!t.fallback&&(t.childs.size>0&&t.activeChilds.size==0||t.childs.size==0&&t.fallbacks.size>0)){let m=t;for(;m.fallbacks.size==0;)if(m=m.parent,!m)return;m&&m.fallbacks.forEach(p=>{if(p.redirect){let H=x("/",p.parent&&p.parent.pattern,p.redirect);f.goto(H,!0);}else p.show();});}}};return setContext("tinro",t),onMount(()=>t.register()),t.router.un=f.subscribe(o=>{t.router.path=o.path,t.router.url=o.url,t.router.query=o.query,t.router.from=o.from,t.pattern!==null&&t.match();}),t}function g(){return getContext("tinro").meta}var f=K();function K(){let{subscribe:e}=writable(l.get(),n=>{l.start(n);let a=T(l.go);return ()=>{l.stop(),a();}});return {subscribe:e,goto:l.go,params:I,meta:g,useHashNavigation:n=>l.mode(n?s.HASH:s.HISTORY),mode:{hash:()=>l.mode(s.HASH),history:()=>l.mode(s.HISTORY),memory:()=>l.mode(s.MEMORY)}}}function A(e){let n=h(e,"href"),a=h(e,"exact",!0),r=h(e,"active-class",!0,"active");return {destroy:f.subscribe(t=>{let o=b(n,t.path);o&&(o.exact&&a||!a)?e.classList.add(r):e.classList.remove(r);})}}function T(e){let n=a=>{let r=a.target.closest("a[href]"),t=r&&h(r,"target",!1,"_self"),o=r&&h(r,"tinro-ignore"),c=a.ctrlKey||a.metaKey||a.altKey||a.shiftKey;if(t=="_self"&&!o&&!c&&r){let i=r.getAttribute("href").replace(/^\/#/,"");/^\/\/|^[a-zA-Z]+:/.test(i)||(a.preventDefault(),e(i.startsWith("/")?i:r.href.replace(window.location.origin,"")));}};return addEventListener("click",n),()=>removeEventListener("click",n)}function I(){return getContext("tinro").meta.params}

    /* node_modules\tinro\cmp\Route.svelte generated by Svelte v3.46.4 */

    const get_default_slot_changes = dirty => ({
    	params: dirty & /*params*/ 2,
    	meta: dirty & /*meta*/ 4
    });

    const get_default_slot_context = ctx => ({
    	params: /*params*/ ctx[1],
    	meta: /*meta*/ ctx[2]
    });

    // (32:0) {#if showContent}
    function create_if_block$9(ctx) {
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
    				if (default_slot.p && (!current || dirty & /*$$scope, params, meta*/ 262)) {
    					update_slot_base(
    						default_slot,
    						default_slot_template,
    						ctx,
    						/*$$scope*/ ctx[8],
    						!current
    						? get_all_dirty_from_scope(/*$$scope*/ ctx[8])
    						: get_slot_changes(default_slot_template, /*$$scope*/ ctx[8], dirty, get_default_slot_changes),
    						get_default_slot_context
    					);
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
    		id: create_if_block$9.name,
    		type: "if",
    		source: "(32:0) {#if showContent}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$i(ctx) {
    	let if_block_anchor;
    	let current;
    	let if_block = /*showContent*/ ctx[0] && create_if_block$9(ctx);

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
    					if_block = create_if_block$9(ctx);
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
    		id: create_fragment$i.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$i($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Route', slots, ['default']);
    	let { path = '/*' } = $$props;
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

    	const writable_props = ['path', 'fallback', 'redirect', 'firstmatch', 'breadcrumb'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Route> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('path' in $$props) $$invalidate(3, path = $$props.path);
    		if ('fallback' in $$props) $$invalidate(4, fallback = $$props.fallback);
    		if ('redirect' in $$props) $$invalidate(5, redirect = $$props.redirect);
    		if ('firstmatch' in $$props) $$invalidate(6, firstmatch = $$props.firstmatch);
    		if ('breadcrumb' in $$props) $$invalidate(7, breadcrumb = $$props.breadcrumb);
    		if ('$$scope' in $$props) $$invalidate(8, $$scope = $$props.$$scope);
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
    		if ('path' in $$props) $$invalidate(3, path = $$props.path);
    		if ('fallback' in $$props) $$invalidate(4, fallback = $$props.fallback);
    		if ('redirect' in $$props) $$invalidate(5, redirect = $$props.redirect);
    		if ('firstmatch' in $$props) $$invalidate(6, firstmatch = $$props.firstmatch);
    		if ('breadcrumb' in $$props) $$invalidate(7, breadcrumb = $$props.breadcrumb);
    		if ('showContent' in $$props) $$invalidate(0, showContent = $$props.showContent);
    		if ('params' in $$props) $$invalidate(1, params = $$props.params);
    		if ('meta' in $$props) $$invalidate(2, meta = $$props.meta);
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

    		init(this, options, instance$i, create_fragment$i, safe_not_equal, {
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
    			id: create_fragment$i.name
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

    /* src\components\Alarm.svelte generated by Svelte v3.46.4 */

    const file$h = "src\\components\\Alarm.svelte";

    // (6:2) {#if title}
    function create_if_block$8(ctx) {
    	let h1;
    	let t;

    	const block = {
    		c: function create() {
    			h1 = element("h1");
    			t = text(/*title*/ ctx[0]);
    			attr_dev(h1, "class", "alm-hdr");
    			add_location(h1, file$h, 6, 4, 82);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, h1, anchor);
    			append_dev(h1, t);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*title*/ 1) set_data_dev(t, /*title*/ ctx[0]);
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(h1);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$8.name,
    		type: "if",
    		source: "(6:2) {#if title}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$h(ctx) {
    	let div;
    	let t;
    	let current;
    	let if_block = /*title*/ ctx[0] && create_if_block$8(ctx);
    	const default_slot_template = /*#slots*/ ctx[2].default;
    	const default_slot = create_slot(default_slot_template, ctx, /*$$scope*/ ctx[1], null);

    	const block = {
    		c: function create() {
    			div = element("div");
    			if (if_block) if_block.c();
    			t = space();
    			if (default_slot) default_slot.c();
    			attr_dev(div, "class", "alm");
    			add_location(div, file$h, 4, 0, 44);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			if (if_block) if_block.m(div, null);
    			append_dev(div, t);

    			if (default_slot) {
    				default_slot.m(div, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*title*/ ctx[0]) {
    				if (if_block) {
    					if_block.p(ctx, dirty);
    				} else {
    					if_block = create_if_block$8(ctx);
    					if_block.c();
    					if_block.m(div, t);
    				}
    			} else if (if_block) {
    				if_block.d(1);
    				if_block = null;
    			}

    			if (default_slot) {
    				if (default_slot.p && (!current || dirty & /*$$scope*/ 2)) {
    					update_slot_base(
    						default_slot,
    						default_slot_template,
    						ctx,
    						/*$$scope*/ ctx[1],
    						!current
    						? get_all_dirty_from_scope(/*$$scope*/ ctx[1])
    						: get_slot_changes(default_slot_template, /*$$scope*/ ctx[1], dirty, null),
    						null
    					);
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
    			if (detaching) detach_dev(div);
    			if (if_block) if_block.d();
    			if (default_slot) default_slot.d(detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$h.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$h($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Alarm', slots, ['default']);
    	let { title } = $$props;
    	const writable_props = ['title'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Alarm> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('title' in $$props) $$invalidate(0, title = $$props.title);
    		if ('$$scope' in $$props) $$invalidate(1, $$scope = $$props.$$scope);
    	};

    	$$self.$capture_state = () => ({ title });

    	$$self.$inject_state = $$props => {
    		if ('title' in $$props) $$invalidate(0, title = $$props.title);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [title, $$scope, slots];
    }

    class Alarm extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$h, create_fragment$h, safe_not_equal, { title: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Alarm",
    			options,
    			id: create_fragment$h.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*title*/ ctx[0] === undefined && !('title' in props)) {
    			console.warn("<Alarm> was created without expected prop 'title'");
    		}
    	}

    	get title() {
    		throw new Error("<Alarm>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set title(value) {
    		throw new Error("<Alarm>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\components\Modal.svelte generated by Svelte v3.46.4 */

    const file$g = "src\\components\\Modal.svelte";

    // (8:2) {#if show}
    function create_if_block$7(ctx) {
    	let div8;
    	let div7;
    	let div0;
    	let t0;
    	let span;
    	let t2;
    	let div6;
    	let div4;
    	let div3;
    	let div2;
    	let h3;
    	let t3;
    	let t4;
    	let div1;
    	let p;
    	let t5;
    	let t6;
    	let div5;
    	let button0;
    	let t8;
    	let button1;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			div8 = element("div");
    			div7 = element("div");
    			div0 = element("div");
    			t0 = space();
    			span = element("span");
    			span.textContent = "​";
    			t2 = space();
    			div6 = element("div");
    			div4 = element("div");
    			div3 = element("div");
    			div2 = element("div");
    			h3 = element("h3");
    			t3 = text(/*header*/ ctx[1]);
    			t4 = space();
    			div1 = element("div");
    			p = element("p");
    			t5 = text(/*text*/ ctx[2]);
    			t6 = space();
    			div5 = element("div");
    			button0 = element("button");
    			button0.textContent = "Deactivate";
    			t8 = space();
    			button1 = element("button");
    			button1.textContent = "Cancel";
    			attr_dev(div0, "class", "fixed inset-0 bg-gray-500 bg-opacity-75 transition-opacity");
    			attr_dev(div0, "aria-hidden", "true");
    			add_location(div0, file$g, 11, 8, 427);
    			attr_dev(span, "class", "hidden sm:inline-block sm:align-middle sm:h-screen");
    			attr_dev(span, "aria-hidden", "true");
    			add_location(span, file$g, 13, 8, 620);
    			attr_dev(h3, "class", "text-lg leading-6 font-medium text-gray-900");
    			attr_dev(h3, "id", "modal-title");
    			add_location(h3, file$g, 18, 16, 1101);
    			attr_dev(p, "class", "text-sm text-gray-500");
    			add_location(p, file$g, 20, 18, 1243);
    			attr_dev(div1, "class", "mt-2");
    			add_location(div1, file$g, 19, 16, 1205);
    			attr_dev(div2, "class", "mt-3 text-center sm:mt-0 sm:ml-4 sm:text-left");
    			add_location(div2, file$g, 17, 14, 1024);
    			attr_dev(div3, "class", "sm:flex sm:items-start");
    			add_location(div3, file$g, 16, 12, 972);
    			attr_dev(div4, "class", "bg-white px-4 pt-5 pb-4 sm:p-6 sm:pb-4");
    			add_location(div4, file$g, 15, 10, 906);
    			attr_dev(button0, "type", "button");
    			attr_dev(button0, "class", "w-full inline-flex justify-center rounded-md border border-transparent shadow-sm px-4 py-2 bg-red-600 text-base font-medium text-white hover:bg-red-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-red-500 sm:ml-3 sm:w-auto sm:text-sm");
    			add_location(button0, file$g, 26, 12, 1466);
    			attr_dev(button1, "type", "button");
    			attr_dev(button1, "class", "mt-3 w-full inline-flex justify-center rounded-md border border-gray-300 shadow-sm px-4 py-2 bg-white text-base font-medium text-gray-700 hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500 sm:mt-0 sm:ml-3 sm:w-auto sm:text-sm");
    			add_location(button1, file$g, 27, 12, 1783);
    			attr_dev(div5, "class", "bg-gray-50 px-4 py-3 sm:px-6 sm:flex sm:flex-row-reverse");
    			add_location(div5, file$g, 25, 10, 1382);
    			attr_dev(div6, "class", "inline-block align-bottom bg-white rounded-lg text-left overflow-hidden shadow-xl transform transition-all sm:my-8 sm:align-middle sm:max-w-lg sm:w-full");
    			add_location(div6, file$g, 14, 8, 728);
    			attr_dev(div7, "class", "flex items-end justify-center min-h-screen pt-4 px-4 pb-20 text-center sm:block sm:p-0");
    			add_location(div7, file$g, 10, 6, 317);
    			attr_dev(div8, "class", "fixed z-10 inset-0 overflow-y-auto");
    			attr_dev(div8, "aria-labelledby", "modal-title");
    			attr_dev(div8, "role", "dialog");
    			attr_dev(div8, "aria-modal", "true");
    			add_location(div8, file$g, 9, 4, 199);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div8, anchor);
    			append_dev(div8, div7);
    			append_dev(div7, div0);
    			append_dev(div7, t0);
    			append_dev(div7, span);
    			append_dev(div7, t2);
    			append_dev(div7, div6);
    			append_dev(div6, div4);
    			append_dev(div4, div3);
    			append_dev(div3, div2);
    			append_dev(div2, h3);
    			append_dev(h3, t3);
    			append_dev(div2, t4);
    			append_dev(div2, div1);
    			append_dev(div1, p);
    			append_dev(p, t5);
    			append_dev(div6, t6);
    			append_dev(div6, div5);
    			append_dev(div5, button0);
    			append_dev(div5, t8);
    			append_dev(div5, button1);

    			if (!mounted) {
    				dispose = listen_dev(button1, "click", /*click_handler*/ ctx[3], false, false, false);
    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*header*/ 2) set_data_dev(t3, /*header*/ ctx[1]);
    			if (dirty & /*text*/ 4) set_data_dev(t5, /*text*/ ctx[2]);
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div8);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$7.name,
    		type: "if",
    		source: "(8:2) {#if show}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$g(ctx) {
    	let div;
    	let if_block = /*show*/ ctx[0] && create_if_block$7(ctx);

    	const block = {
    		c: function create() {
    			div = element("div");
    			if (if_block) if_block.c();
    			attr_dev(div, "class", "modal");
    			add_location(div, file$g, 6, 0, 105);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			if (if_block) if_block.m(div, null);
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*show*/ ctx[0]) {
    				if (if_block) {
    					if_block.p(ctx, dirty);
    				} else {
    					if_block = create_if_block$7(ctx);
    					if_block.c();
    					if_block.m(div, null);
    				}
    			} else if (if_block) {
    				if_block.d(1);
    				if_block = null;
    			}
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			if (if_block) if_block.d();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$g.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$g($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Modal', slots, []);
    	let { show } = $$props;
    	let { header = "header" } = $$props;
    	let { text = "text" } = $$props;
    	const writable_props = ['show', 'header', 'text'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Modal> was created with unknown prop '${key}'`);
    	});

    	const click_handler = () => $$invalidate(0, show = false);

    	$$self.$$set = $$props => {
    		if ('show' in $$props) $$invalidate(0, show = $$props.show);
    		if ('header' in $$props) $$invalidate(1, header = $$props.header);
    		if ('text' in $$props) $$invalidate(2, text = $$props.text);
    	};

    	$$self.$capture_state = () => ({ show, header, text });

    	$$self.$inject_state = $$props => {
    		if ('show' in $$props) $$invalidate(0, show = $$props.show);
    		if ('header' in $$props) $$invalidate(1, header = $$props.header);
    		if ('text' in $$props) $$invalidate(2, text = $$props.text);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [show, header, text, click_handler];
    }

    class Modal extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$g, create_fragment$g, safe_not_equal, { show: 0, header: 1, text: 2 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Modal",
    			options,
    			id: create_fragment$g.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*show*/ ctx[0] === undefined && !('show' in props)) {
    			console.warn("<Modal> was created without expected prop 'show'");
    		}
    	}

    	get show() {
    		throw new Error("<Modal>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set show(value) {
    		throw new Error("<Modal>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get header() {
    		throw new Error("<Modal>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set header(value) {
    		throw new Error("<Modal>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get text() {
    		throw new Error("<Modal>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set text(value) {
    		throw new Error("<Modal>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\components\Card.svelte generated by Svelte v3.46.4 */

    const file$f = "src\\components\\Card.svelte";

    // (6:2) {#if title}
    function create_if_block$6(ctx) {
    	let h1;
    	let t;

    	const block = {
    		c: function create() {
    			h1 = element("h1");
    			t = text(/*title*/ ctx[0]);
    			attr_dev(h1, "class", "crd-hdr");
    			add_location(h1, file$f, 6, 4, 82);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, h1, anchor);
    			append_dev(h1, t);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*title*/ 1) set_data_dev(t, /*title*/ ctx[0]);
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(h1);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$6.name,
    		type: "if",
    		source: "(6:2) {#if title}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$f(ctx) {
    	let div;
    	let t;
    	let current;
    	let if_block = /*title*/ ctx[0] && create_if_block$6(ctx);
    	const default_slot_template = /*#slots*/ ctx[2].default;
    	const default_slot = create_slot(default_slot_template, ctx, /*$$scope*/ ctx[1], null);

    	const block = {
    		c: function create() {
    			div = element("div");
    			if (if_block) if_block.c();
    			t = space();
    			if (default_slot) default_slot.c();
    			attr_dev(div, "class", "crd");
    			add_location(div, file$f, 4, 0, 44);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			if (if_block) if_block.m(div, null);
    			append_dev(div, t);

    			if (default_slot) {
    				default_slot.m(div, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*title*/ ctx[0]) {
    				if (if_block) {
    					if_block.p(ctx, dirty);
    				} else {
    					if_block = create_if_block$6(ctx);
    					if_block.c();
    					if_block.m(div, t);
    				}
    			} else if (if_block) {
    				if_block.d(1);
    				if_block = null;
    			}

    			if (default_slot) {
    				if (default_slot.p && (!current || dirty & /*$$scope*/ 2)) {
    					update_slot_base(
    						default_slot,
    						default_slot_template,
    						ctx,
    						/*$$scope*/ ctx[1],
    						!current
    						? get_all_dirty_from_scope(/*$$scope*/ ctx[1])
    						: get_slot_changes(default_slot_template, /*$$scope*/ ctx[1], dirty, null),
    						null
    					);
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
    			if (detaching) detach_dev(div);
    			if (if_block) if_block.d();
    			if (default_slot) default_slot.d(detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$f.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$f($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Card', slots, ['default']);
    	let { title } = $$props;
    	const writable_props = ['title'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Card> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('title' in $$props) $$invalidate(0, title = $$props.title);
    		if ('$$scope' in $$props) $$invalidate(1, $$scope = $$props.$$scope);
    	};

    	$$self.$capture_state = () => ({ title });

    	$$self.$inject_state = $$props => {
    		if ('title' in $$props) $$invalidate(0, title = $$props.title);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [title, $$scope, slots];
    }

    class Card extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$f, create_fragment$f, safe_not_equal, { title: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Card",
    			options,
    			id: create_fragment$f.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*title*/ ctx[0] === undefined && !('title' in props)) {
    			console.warn("<Card> was created without expected prop 'title'");
    		}
    	}

    	get title() {
    		throw new Error("<Card>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set title(value) {
    		throw new Error("<Card>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\widgets\Input.svelte generated by Svelte v3.46.4 */

    const file$e = "src\\widgets\\Input.svelte";

    // (14:4) {#if widget.type == "number"}
    function create_if_block_3$2(ctx) {
    	let input;
    	let input_class_value;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			input = element("input");

    			attr_dev(input, "class", input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500");

    			attr_dev(input, "step", "0.1");
    			attr_dev(input, "type", "number");
    			add_location(input, file$e, 14, 6, 402);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, input, anchor);
    			set_input_value(input, /*widget*/ ctx[0].status);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input, "change", /*change_handler*/ ctx[3], false, false, false),
    					listen_dev(input, "input", /*input_input_handler*/ ctx[4])
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*widget*/ 1 && input_class_value !== (input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500")) {
    				attr_dev(input, "class", input_class_value);
    			}

    			if (dirty & /*widget*/ 1 && to_number(input.value) !== /*widget*/ ctx[0].status) {
    				set_input_value(input, /*widget*/ ctx[0].status);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(input);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_3$2.name,
    		type: "if",
    		source: "(14:4) {#if widget.type == \\\"number\\\"}",
    		ctx
    	});

    	return block;
    }

    // (17:4) {#if widget.type == "text"}
    function create_if_block_2$2(ctx) {
    	let input;
    	let input_class_value;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			input = element("input");

    			attr_dev(input, "class", input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500");

    			attr_dev(input, "type", "text");
    			add_location(input, file$e, 17, 6, 723);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, input, anchor);
    			set_input_value(input, /*widget*/ ctx[0].status);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input, "change", /*change_handler_1*/ ctx[5], false, false, false),
    					listen_dev(input, "input", /*input_input_handler_1*/ ctx[6])
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*widget*/ 1 && input_class_value !== (input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500")) {
    				attr_dev(input, "class", input_class_value);
    			}

    			if (dirty & /*widget*/ 1 && input.value !== /*widget*/ ctx[0].status) {
    				set_input_value(input, /*widget*/ ctx[0].status);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(input);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_2$2.name,
    		type: "if",
    		source: "(17:4) {#if widget.type == \\\"text\\\"}",
    		ctx
    	});

    	return block;
    }

    // (20:4) {#if widget.type == "date"}
    function create_if_block_1$2(ctx) {
    	let input;
    	let input_class_value;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			input = element("input");

    			attr_dev(input, "class", input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500");

    			attr_dev(input, "type", "date");
    			add_location(input, file$e, 20, 6, 1031);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, input, anchor);
    			set_input_value(input, /*widget*/ ctx[0].status);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input, "change", /*change_handler_2*/ ctx[7], false, false, false),
    					listen_dev(input, "input", /*input_input_handler_2*/ ctx[8])
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*widget*/ 1 && input_class_value !== (input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500")) {
    				attr_dev(input, "class", input_class_value);
    			}

    			if (dirty & /*widget*/ 1) {
    				set_input_value(input, /*widget*/ ctx[0].status);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(input);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_1$2.name,
    		type: "if",
    		source: "(20:4) {#if widget.type == \\\"date\\\"}",
    		ctx
    	});

    	return block;
    }

    // (23:4) {#if widget.type == "time"}
    function create_if_block$5(ctx) {
    	let input;
    	let input_class_value;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			input = element("input");

    			attr_dev(input, "class", input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500");

    			attr_dev(input, "type", "time");
    			add_location(input, file$e, 23, 6, 1339);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, input, anchor);
    			set_input_value(input, /*widget*/ ctx[0].status);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input, "change", /*change_handler_3*/ ctx[9], false, false, false),
    					listen_dev(input, "input", /*input_input_handler_3*/ ctx[10])
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*widget*/ 1 && input_class_value !== (input_class_value = /*widget*/ ctx[0]["send"] == true
    			? "ipt-rnd text-right border-red-500"
    			: "ipt-rnd text-right focus:border-indigo-500")) {
    				attr_dev(input, "class", input_class_value);
    			}

    			if (dirty & /*widget*/ 1) {
    				set_input_value(input, /*widget*/ ctx[0].status);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(input);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$5.name,
    		type: "if",
    		source: "(23:4) {#if widget.type == \\\"time\\\"}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$e(ctx) {
    	let div2;
    	let div0;
    	let label;
    	let t0_value = (!/*widget*/ ctx[0].descr ? "" : /*widget*/ ctx[0].descr) + "";
    	let t0;
    	let t1;
    	let div1;
    	let t2;
    	let t3;
    	let t4;
    	let if_block0 = /*widget*/ ctx[0].type == "number" && create_if_block_3$2(ctx);
    	let if_block1 = /*widget*/ ctx[0].type == "text" && create_if_block_2$2(ctx);
    	let if_block2 = /*widget*/ ctx[0].type == "date" && create_if_block_1$2(ctx);
    	let if_block3 = /*widget*/ ctx[0].type == "time" && create_if_block$5(ctx);

    	const block = {
    		c: function create() {
    			div2 = element("div");
    			div0 = element("div");
    			label = element("label");
    			t0 = text(t0_value);
    			t1 = space();
    			div1 = element("div");
    			if (if_block0) if_block0.c();
    			t2 = space();
    			if (if_block1) if_block1.c();
    			t3 = space();
    			if (if_block2) if_block2.c();
    			t4 = space();
    			if (if_block3) if_block3.c();
    			attr_dev(label, "class", "wgt-dscr-stl");
    			add_location(label, file$e, 10, 4, 255);
    			attr_dev(div0, "class", "wgt-dscr-w");
    			add_location(div0, file$e, 8, 2, 163);
    			attr_dev(div1, "class", "wgt-w");
    			add_location(div1, file$e, 12, 2, 340);
    			attr_dev(div2, "class", "crd-itm-psn");
    			add_location(div2, file$e, 7, 0, 134);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div0);
    			append_dev(div0, label);
    			append_dev(label, t0);
    			append_dev(div2, t1);
    			append_dev(div2, div1);
    			if (if_block0) if_block0.m(div1, null);
    			append_dev(div1, t2);
    			if (if_block1) if_block1.m(div1, null);
    			append_dev(div1, t3);
    			if (if_block2) if_block2.m(div1, null);
    			append_dev(div1, t4);
    			if (if_block3) if_block3.m(div1, null);
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*widget*/ 1 && t0_value !== (t0_value = (!/*widget*/ ctx[0].descr ? "" : /*widget*/ ctx[0].descr) + "")) set_data_dev(t0, t0_value);

    			if (/*widget*/ ctx[0].type == "number") {
    				if (if_block0) {
    					if_block0.p(ctx, dirty);
    				} else {
    					if_block0 = create_if_block_3$2(ctx);
    					if_block0.c();
    					if_block0.m(div1, t2);
    				}
    			} else if (if_block0) {
    				if_block0.d(1);
    				if_block0 = null;
    			}

    			if (/*widget*/ ctx[0].type == "text") {
    				if (if_block1) {
    					if_block1.p(ctx, dirty);
    				} else {
    					if_block1 = create_if_block_2$2(ctx);
    					if_block1.c();
    					if_block1.m(div1, t3);
    				}
    			} else if (if_block1) {
    				if_block1.d(1);
    				if_block1 = null;
    			}

    			if (/*widget*/ ctx[0].type == "date") {
    				if (if_block2) {
    					if_block2.p(ctx, dirty);
    				} else {
    					if_block2 = create_if_block_1$2(ctx);
    					if_block2.c();
    					if_block2.m(div1, t4);
    				}
    			} else if (if_block2) {
    				if_block2.d(1);
    				if_block2 = null;
    			}

    			if (/*widget*/ ctx[0].type == "time") {
    				if (if_block3) {
    					if_block3.p(ctx, dirty);
    				} else {
    					if_block3 = create_if_block$5(ctx);
    					if_block3.c();
    					if_block3.m(div1, null);
    				}
    			} else if (if_block3) {
    				if_block3.d(1);
    				if_block3 = null;
    			}
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div2);
    			if (if_block0) if_block0.d();
    			if (if_block1) if_block1.d();
    			if (if_block2) if_block2.d();
    			if (if_block3) if_block3.d();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$e.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$e($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Input', slots, []);
    	let { widget } = $$props;

    	let { wsPush = (ws, topic, status) => {
    		
    	} } = $$props;

    	let { value } = $$props;
    	value = value;
    	const writable_props = ['widget', 'wsPush', 'value'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Input> was created with unknown prop '${key}'`);
    	});

    	const change_handler = () => ($$invalidate(0, widget["send"] = true, widget), wsPush(widget.ws, widget.topic, widget.status));

    	function input_input_handler() {
    		widget.status = to_number(this.value);
    		$$invalidate(0, widget);
    	}

    	const change_handler_1 = () => ($$invalidate(0, widget["send"] = true, widget), wsPush(widget.ws, widget.topic, widget.status));

    	function input_input_handler_1() {
    		widget.status = this.value;
    		$$invalidate(0, widget);
    	}

    	const change_handler_2 = () => ($$invalidate(0, widget["send"] = true, widget), wsPush(widget.ws, widget.topic, widget.status));

    	function input_input_handler_2() {
    		widget.status = this.value;
    		$$invalidate(0, widget);
    	}

    	const change_handler_3 = () => ($$invalidate(0, widget["send"] = true, widget), wsPush(widget.ws, widget.topic, widget.status));

    	function input_input_handler_3() {
    		widget.status = this.value;
    		$$invalidate(0, widget);
    	}

    	$$self.$$set = $$props => {
    		if ('widget' in $$props) $$invalidate(0, widget = $$props.widget);
    		if ('wsPush' in $$props) $$invalidate(1, wsPush = $$props.wsPush);
    		if ('value' in $$props) $$invalidate(2, value = $$props.value);
    	};

    	$$self.$capture_state = () => ({ widget, wsPush, value });

    	$$self.$inject_state = $$props => {
    		if ('widget' in $$props) $$invalidate(0, widget = $$props.widget);
    		if ('wsPush' in $$props) $$invalidate(1, wsPush = $$props.wsPush);
    		if ('value' in $$props) $$invalidate(2, value = $$props.value);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		widget,
    		wsPush,
    		value,
    		change_handler,
    		input_input_handler,
    		change_handler_1,
    		input_input_handler_1,
    		change_handler_2,
    		input_input_handler_2,
    		change_handler_3,
    		input_input_handler_3
    	];
    }

    class Input extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$e, create_fragment$e, safe_not_equal, { widget: 0, wsPush: 1, value: 2 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Input",
    			options,
    			id: create_fragment$e.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*widget*/ ctx[0] === undefined && !('widget' in props)) {
    			console.warn("<Input> was created without expected prop 'widget'");
    		}

    		if (/*value*/ ctx[2] === undefined && !('value' in props)) {
    			console.warn("<Input> was created without expected prop 'value'");
    		}
    	}

    	get widget() {
    		throw new Error("<Input>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set widget(value) {
    		throw new Error("<Input>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get wsPush() {
    		throw new Error("<Input>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set wsPush(value) {
    		throw new Error("<Input>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get value() {
    		throw new Error("<Input>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set value(value) {
    		throw new Error("<Input>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\widgets\Toggle.svelte generated by Svelte v3.46.4 */

    const file$d = "src\\widgets\\Toggle.svelte";

    function create_fragment$d(ctx) {
    	let div5;
    	let div0;
    	let label0;
    	let t0_value = (!/*widget*/ ctx[0].descr ? "" : /*widget*/ ctx[0].descr) + "";
    	let t0;
    	let t1;
    	let div4;
    	let label1;
    	let div3;
    	let input;
    	let input_id_value;
    	let t2;
    	let div1;
    	let t3;
    	let div2;
    	let div2_class_value;
    	let label1_for_value;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			div5 = element("div");
    			div0 = element("div");
    			label0 = element("label");
    			t0 = text(t0_value);
    			t1 = space();
    			div4 = element("div");
    			label1 = element("label");
    			div3 = element("div");
    			input = element("input");
    			t2 = space();
    			div1 = element("div");
    			t3 = space();
    			div2 = element("div");
    			attr_dev(label0, "class", "wgt-dscr-stl");
    			add_location(label0, file$d, 10, 4, 255);
    			attr_dev(div0, "class", "wgt-dscr-w");
    			add_location(div0, file$d, 8, 2, 163);
    			attr_dev(input, "id", input_id_value = /*widget*/ ctx[0].topic);
    			attr_dev(input, "type", "checkbox");
    			attr_dev(input, "class", "sr-only");
    			add_location(input, file$d, 15, 8, 467);
    			attr_dev(div1, "class", "block bg-gray-600 w-10 h-6 rounded-full");
    			add_location(div1, file$d, 16, 8, 655);

    			attr_dev(div2, "class", div2_class_value = "dot " + (/*widget*/ ctx[0]['send'] == true
    			? 'bg-red-400'
    			: 'bg-white') + " absolute left-1 top-1 w-4 h-4 rounded-full transition");

    			add_location(div2, file$d, 17, 8, 720);
    			attr_dev(div3, "class", "relative");
    			add_location(div3, file$d, 14, 6, 435);
    			attr_dev(label1, "for", label1_for_value = /*widget*/ ctx[0].topic);
    			attr_dev(label1, "class", "items-center cursor-pointer");
    			add_location(label1, file$d, 13, 4, 365);
    			attr_dev(div4, "class", "wgt-w");
    			add_location(div4, file$d, 12, 2, 340);
    			attr_dev(div5, "class", "crd-itm-psn");
    			add_location(div5, file$d, 7, 0, 134);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div5, anchor);
    			append_dev(div5, div0);
    			append_dev(div0, label0);
    			append_dev(label0, t0);
    			append_dev(div5, t1);
    			append_dev(div5, div4);
    			append_dev(div4, label1);
    			append_dev(label1, div3);
    			append_dev(div3, input);
    			set_input_value(input, /*widget*/ ctx[0].status);
    			append_dev(div3, t2);
    			append_dev(div3, div1);
    			append_dev(div3, t3);
    			append_dev(div3, div2);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input, "change", /*change_handler*/ ctx[3], false, false, false),
    					listen_dev(input, "change", /*input_change_handler*/ ctx[4])
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*widget*/ 1 && t0_value !== (t0_value = (!/*widget*/ ctx[0].descr ? "" : /*widget*/ ctx[0].descr) + "")) set_data_dev(t0, t0_value);

    			if (dirty & /*widget*/ 1 && input_id_value !== (input_id_value = /*widget*/ ctx[0].topic)) {
    				attr_dev(input, "id", input_id_value);
    			}

    			if (dirty & /*widget*/ 1) {
    				set_input_value(input, /*widget*/ ctx[0].status);
    			}

    			if (dirty & /*widget*/ 1 && div2_class_value !== (div2_class_value = "dot " + (/*widget*/ ctx[0]['send'] == true
    			? 'bg-red-400'
    			: 'bg-white') + " absolute left-1 top-1 w-4 h-4 rounded-full transition")) {
    				attr_dev(div2, "class", div2_class_value);
    			}

    			if (dirty & /*widget*/ 1 && label1_for_value !== (label1_for_value = /*widget*/ ctx[0].topic)) {
    				attr_dev(label1, "for", label1_for_value);
    			}
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div5);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$d.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$d($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Toggle', slots, []);
    	let { widget } = $$props;
    	let { value } = $$props;
    	value = value;

    	let { wsPush = (ws, topic, status) => {
    		
    	} } = $$props;

    	const writable_props = ['widget', 'value', 'wsPush'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Toggle> was created with unknown prop '${key}'`);
    	});

    	const change_handler = () => ($$invalidate(0, widget["send"] = true, widget), wsPush(widget.ws, widget.topic, widget.status));

    	function input_change_handler() {
    		widget.status = this.value;
    		$$invalidate(0, widget);
    	}

    	$$self.$$set = $$props => {
    		if ('widget' in $$props) $$invalidate(0, widget = $$props.widget);
    		if ('value' in $$props) $$invalidate(2, value = $$props.value);
    		if ('wsPush' in $$props) $$invalidate(1, wsPush = $$props.wsPush);
    	};

    	$$self.$capture_state = () => ({ widget, value, wsPush });

    	$$self.$inject_state = $$props => {
    		if ('widget' in $$props) $$invalidate(0, widget = $$props.widget);
    		if ('value' in $$props) $$invalidate(2, value = $$props.value);
    		if ('wsPush' in $$props) $$invalidate(1, wsPush = $$props.wsPush);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [widget, wsPush, value, change_handler, input_change_handler];
    }

    class Toggle extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$d, create_fragment$d, safe_not_equal, { widget: 0, value: 2, wsPush: 1 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Toggle",
    			options,
    			id: create_fragment$d.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*widget*/ ctx[0] === undefined && !('widget' in props)) {
    			console.warn("<Toggle> was created without expected prop 'widget'");
    		}

    		if (/*value*/ ctx[2] === undefined && !('value' in props)) {
    			console.warn("<Toggle> was created without expected prop 'value'");
    		}
    	}

    	get widget() {
    		throw new Error("<Toggle>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set widget(value) {
    		throw new Error("<Toggle>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get value() {
    		throw new Error("<Toggle>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set value(value) {
    		throw new Error("<Toggle>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get wsPush() {
    		throw new Error("<Toggle>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set wsPush(value) {
    		throw new Error("<Toggle>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\widgets\Anydata.svelte generated by Svelte v3.46.4 */

    const file$c = "src\\widgets\\Anydata.svelte";

    function create_fragment$c(ctx) {
    	let div2;
    	let div0;
    	let label0;
    	let t0_value = (!/*widget*/ ctx[0].descr ? "" : /*widget*/ ctx[0].descr) + "";
    	let t0;
    	let t1;
    	let div1;
    	let label1;

    	let t2_value = (!/*widget*/ ctx[0].status
    	? ""
    	: /*widget*/ ctx[0].status) + "";

    	let t2;
    	let t3;
    	let label2;
    	let t4;
    	let t5_value = (!/*widget*/ ctx[0].after ? "" : /*widget*/ ctx[0].after) + "";
    	let t5;

    	const block = {
    		c: function create() {
    			div2 = element("div");
    			div0 = element("div");
    			label0 = element("label");
    			t0 = text(t0_value);
    			t1 = space();
    			div1 = element("div");
    			label1 = element("label");
    			t2 = text(t2_value);
    			t3 = space();
    			label2 = element("label");
    			t4 = text(" ");
    			t5 = text(t5_value);
    			attr_dev(label0, "class", "wgt-dscr-stl");
    			add_location(label0, file$c, 9, 4, 205);
    			attr_dev(div0, "class", "wgt-dscr-w");
    			add_location(div0, file$c, 7, 2, 113);
    			attr_dev(label1, "class", "wgt-adt-stl");
    			add_location(label1, file$c, 13, 4, 377);
    			attr_dev(label2, "class", "wgt-adt-stl");
    			add_location(label2, file$c, 15, 4, 517);
    			attr_dev(div1, "class", "wgt-w");
    			add_location(div1, file$c, 11, 2, 290);
    			attr_dev(div2, "class", "crd-itm-psn");
    			add_location(div2, file$c, 6, 0, 84);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div0);
    			append_dev(div0, label0);
    			append_dev(label0, t0);
    			append_dev(div2, t1);
    			append_dev(div2, div1);
    			append_dev(div1, label1);
    			append_dev(label1, t2);
    			append_dev(div1, t3);
    			append_dev(div1, label2);
    			append_dev(label2, t4);
    			append_dev(label2, t5);
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*widget*/ 1 && t0_value !== (t0_value = (!/*widget*/ ctx[0].descr ? "" : /*widget*/ ctx[0].descr) + "")) set_data_dev(t0, t0_value);

    			if (dirty & /*widget*/ 1 && t2_value !== (t2_value = (!/*widget*/ ctx[0].status
    			? ""
    			: /*widget*/ ctx[0].status) + "")) set_data_dev(t2, t2_value);

    			if (dirty & /*widget*/ 1 && t5_value !== (t5_value = (!/*widget*/ ctx[0].after ? "" : /*widget*/ ctx[0].after) + "")) set_data_dev(t5, t5_value);
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div2);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$c.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$c($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Anydata', slots, []);
    	let { widget } = $$props;
    	let { value } = $$props;
    	value = value;
    	const writable_props = ['widget', 'value'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Anydata> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('widget' in $$props) $$invalidate(0, widget = $$props.widget);
    		if ('value' in $$props) $$invalidate(1, value = $$props.value);
    	};

    	$$self.$capture_state = () => ({ widget, value });

    	$$self.$inject_state = $$props => {
    		if ('widget' in $$props) $$invalidate(0, widget = $$props.widget);
    		if ('value' in $$props) $$invalidate(1, value = $$props.value);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [widget, value];
    }

    class Anydata extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$c, create_fragment$c, safe_not_equal, { widget: 0, value: 1 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Anydata",
    			options,
    			id: create_fragment$c.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*widget*/ ctx[0] === undefined && !('widget' in props)) {
    			console.warn("<Anydata> was created without expected prop 'widget'");
    		}

    		if (/*value*/ ctx[1] === undefined && !('value' in props)) {
    			console.warn("<Anydata> was created without expected prop 'value'");
    		}
    	}

    	get widget() {
    		throw new Error("<Anydata>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set widget(value) {
    		throw new Error("<Anydata>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get value() {
    		throw new Error("<Anydata>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set value(value) {
    		throw new Error("<Anydata>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\pages\Dashboard.svelte generated by Svelte v3.46.4 */
    const file$b = "src\\pages\\Dashboard.svelte";

    function get_each_context$5(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[8] = list[i];
    	child_ctx[10] = i;
    	return child_ctx;
    }

    function get_each_context_1$1(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[11] = list[i];
    	child_ctx[12] = list;
    	child_ctx[10] = i;
    	return child_ctx;
    }

    // (14:2) {#if layoutJson === []}
    function create_if_block_4(ctx) {
    	let card;
    	let current;

    	card = new Card({
    			props: {
    				title: "Ваша панель управления пуста, вначале добавьте новые элементы в конфигураторе!"
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(card.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(card, target, anchor);
    			current = true;
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(card.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(card.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(card, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_4.name,
    		type: "if",
    		source: "(14:2) {#if layoutJson === []}",
    		ctx
    	});

    	return block;
    }

    // (20:8) {#if widget.page === pagesName.page}
    function create_if_block$4(ctx) {
    	let t0;
    	let t1;
    	let if_block2_anchor;
    	let current;
    	let if_block0 = /*widget*/ ctx[11].widget === "input" && create_if_block_3$1(ctx);
    	let if_block1 = /*widget*/ ctx[11].widget === "toggle" && create_if_block_2$1(ctx);
    	let if_block2 = /*widget*/ ctx[11].widget === "anydata" && create_if_block_1$1(ctx);

    	const block = {
    		c: function create() {
    			if (if_block0) if_block0.c();
    			t0 = space();
    			if (if_block1) if_block1.c();
    			t1 = space();
    			if (if_block2) if_block2.c();
    			if_block2_anchor = empty();
    		},
    		m: function mount(target, anchor) {
    			if (if_block0) if_block0.m(target, anchor);
    			insert_dev(target, t0, anchor);
    			if (if_block1) if_block1.m(target, anchor);
    			insert_dev(target, t1, anchor);
    			if (if_block2) if_block2.m(target, anchor);
    			insert_dev(target, if_block2_anchor, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (/*widget*/ ctx[11].widget === "input") {
    				if (if_block0) {
    					if_block0.p(ctx, dirty);

    					if (dirty & /*layoutJson*/ 1) {
    						transition_in(if_block0, 1);
    					}
    				} else {
    					if_block0 = create_if_block_3$1(ctx);
    					if_block0.c();
    					transition_in(if_block0, 1);
    					if_block0.m(t0.parentNode, t0);
    				}
    			} else if (if_block0) {
    				group_outros();

    				transition_out(if_block0, 1, 1, () => {
    					if_block0 = null;
    				});

    				check_outros();
    			}

    			if (/*widget*/ ctx[11].widget === "toggle") {
    				if (if_block1) {
    					if_block1.p(ctx, dirty);

    					if (dirty & /*layoutJson*/ 1) {
    						transition_in(if_block1, 1);
    					}
    				} else {
    					if_block1 = create_if_block_2$1(ctx);
    					if_block1.c();
    					transition_in(if_block1, 1);
    					if_block1.m(t1.parentNode, t1);
    				}
    			} else if (if_block1) {
    				group_outros();

    				transition_out(if_block1, 1, 1, () => {
    					if_block1 = null;
    				});

    				check_outros();
    			}

    			if (/*widget*/ ctx[11].widget === "anydata") {
    				if (if_block2) {
    					if_block2.p(ctx, dirty);

    					if (dirty & /*layoutJson*/ 1) {
    						transition_in(if_block2, 1);
    					}
    				} else {
    					if_block2 = create_if_block_1$1(ctx);
    					if_block2.c();
    					transition_in(if_block2, 1);
    					if_block2.m(if_block2_anchor.parentNode, if_block2_anchor);
    				}
    			} else if (if_block2) {
    				group_outros();

    				transition_out(if_block2, 1, 1, () => {
    					if_block2 = null;
    				});

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(if_block0);
    			transition_in(if_block1);
    			transition_in(if_block2);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(if_block0);
    			transition_out(if_block1);
    			transition_out(if_block2);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (if_block0) if_block0.d(detaching);
    			if (detaching) detach_dev(t0);
    			if (if_block1) if_block1.d(detaching);
    			if (detaching) detach_dev(t1);
    			if (if_block2) if_block2.d(detaching);
    			if (detaching) detach_dev(if_block2_anchor);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$4.name,
    		type: "if",
    		source: "(20:8) {#if widget.page === pagesName.page}",
    		ctx
    	});

    	return block;
    }

    // (21:10) {#if widget.widget === "input"}
    function create_if_block_3$1(ctx) {
    	let input;
    	let updating_value;
    	let current;

    	function input_value_binding(value) {
    		/*input_value_binding*/ ctx[4](value, /*widget*/ ctx[11]);
    	}

    	let input_props = {
    		widget: /*widget*/ ctx[11],
    		wsPush: /*func*/ ctx[3]
    	};

    	if (/*widget*/ ctx[11].status !== void 0) {
    		input_props.value = /*widget*/ ctx[11].status;
    	}

    	input = new Input({ props: input_props, $$inline: true });
    	binding_callbacks.push(() => bind(input, 'value', input_value_binding));

    	const block = {
    		c: function create() {
    			create_component(input.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(input, target, anchor);
    			current = true;
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    			const input_changes = {};
    			if (dirty & /*layoutJson*/ 1) input_changes.widget = /*widget*/ ctx[11];
    			if (dirty & /*wsPush*/ 4) input_changes.wsPush = /*func*/ ctx[3];

    			if (!updating_value && dirty & /*layoutJson*/ 1) {
    				updating_value = true;
    				input_changes.value = /*widget*/ ctx[11].status;
    				add_flush_callback(() => updating_value = false);
    			}

    			input.$set(input_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(input.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(input.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(input, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_3$1.name,
    		type: "if",
    		source: "(21:10) {#if widget.widget === \\\"input\\\"}",
    		ctx
    	});

    	return block;
    }

    // (24:10) {#if widget.widget === "toggle"}
    function create_if_block_2$1(ctx) {
    	let toggle;
    	let updating_value;
    	let current;

    	function toggle_value_binding(value) {
    		/*toggle_value_binding*/ ctx[6](value, /*widget*/ ctx[11]);
    	}

    	let toggle_props = {
    		widget: /*widget*/ ctx[11],
    		wsPush: /*func_1*/ ctx[5]
    	};

    	if (/*widget*/ ctx[11].status !== void 0) {
    		toggle_props.value = /*widget*/ ctx[11].status;
    	}

    	toggle = new Toggle({ props: toggle_props, $$inline: true });
    	binding_callbacks.push(() => bind(toggle, 'value', toggle_value_binding));

    	const block = {
    		c: function create() {
    			create_component(toggle.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(toggle, target, anchor);
    			current = true;
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    			const toggle_changes = {};
    			if (dirty & /*layoutJson*/ 1) toggle_changes.widget = /*widget*/ ctx[11];
    			if (dirty & /*wsPush*/ 4) toggle_changes.wsPush = /*func_1*/ ctx[5];

    			if (!updating_value && dirty & /*layoutJson*/ 1) {
    				updating_value = true;
    				toggle_changes.value = /*widget*/ ctx[11].status;
    				add_flush_callback(() => updating_value = false);
    			}

    			toggle.$set(toggle_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(toggle.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(toggle.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(toggle, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_2$1.name,
    		type: "if",
    		source: "(24:10) {#if widget.widget === \\\"toggle\\\"}",
    		ctx
    	});

    	return block;
    }

    // (27:10) {#if widget.widget === "anydata"}
    function create_if_block_1$1(ctx) {
    	let anydata;
    	let updating_value;
    	let current;

    	function anydata_value_binding(value) {
    		/*anydata_value_binding*/ ctx[7](value, /*widget*/ ctx[11]);
    	}

    	let anydata_props = { widget: /*widget*/ ctx[11] };

    	if (/*widget*/ ctx[11].status !== void 0) {
    		anydata_props.value = /*widget*/ ctx[11].status;
    	}

    	anydata = new Anydata({ props: anydata_props, $$inline: true });
    	binding_callbacks.push(() => bind(anydata, 'value', anydata_value_binding));

    	const block = {
    		c: function create() {
    			create_component(anydata.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(anydata, target, anchor);
    			current = true;
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    			const anydata_changes = {};
    			if (dirty & /*layoutJson*/ 1) anydata_changes.widget = /*widget*/ ctx[11];

    			if (!updating_value && dirty & /*layoutJson*/ 1) {
    				updating_value = true;
    				anydata_changes.value = /*widget*/ ctx[11].status;
    				add_flush_callback(() => updating_value = false);
    			}

    			anydata.$set(anydata_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(anydata.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(anydata.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(anydata, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_1$1.name,
    		type: "if",
    		source: "(27:10) {#if widget.widget === \\\"anydata\\\"}",
    		ctx
    	});

    	return block;
    }

    // (19:6) {#each layoutJson as widget, i}
    function create_each_block_1$1(ctx) {
    	let if_block_anchor;
    	let current;
    	let if_block = /*widget*/ ctx[11].page === /*pagesName*/ ctx[8].page && create_if_block$4(ctx);

    	const block = {
    		c: function create() {
    			if (if_block) if_block.c();
    			if_block_anchor = empty();
    		},
    		m: function mount(target, anchor) {
    			if (if_block) if_block.m(target, anchor);
    			insert_dev(target, if_block_anchor, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (/*widget*/ ctx[11].page === /*pagesName*/ ctx[8].page) {
    				if (if_block) {
    					if_block.p(ctx, dirty);

    					if (dirty & /*layoutJson, pages*/ 3) {
    						transition_in(if_block, 1);
    					}
    				} else {
    					if_block = create_if_block$4(ctx);
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
    		id: create_each_block_1$1.name,
    		type: "each",
    		source: "(19:6) {#each layoutJson as widget, i}",
    		ctx
    	});

    	return block;
    }

    // (18:4) <Card title={pagesName.page}>
    function create_default_slot$7(ctx) {
    	let t;
    	let current;
    	let each_value_1 = /*layoutJson*/ ctx[0];
    	validate_each_argument(each_value_1);
    	let each_blocks = [];

    	for (let i = 0; i < each_value_1.length; i += 1) {
    		each_blocks[i] = create_each_block_1$1(get_each_context_1$1(ctx, each_value_1, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			t = space();
    		},
    		m: function mount(target, anchor) {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(target, anchor);
    			}

    			insert_dev(target, t, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*layoutJson, wsPush, pages*/ 7) {
    				each_value_1 = /*layoutJson*/ ctx[0];
    				validate_each_argument(each_value_1);
    				let i;

    				for (i = 0; i < each_value_1.length; i += 1) {
    					const child_ctx = get_each_context_1$1(ctx, each_value_1, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block_1$1(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(t.parentNode, t);
    					}
    				}

    				group_outros();

    				for (i = each_value_1.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value_1.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_each(each_blocks, detaching);
    			if (detaching) detach_dev(t);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot$7.name,
    		type: "slot",
    		source: "(18:4) <Card title={pagesName.page}>",
    		ctx
    	});

    	return block;
    }

    // (17:2) {#each pages as pagesName, i}
    function create_each_block$5(ctx) {
    	let card;
    	let current;

    	card = new Card({
    			props: {
    				title: /*pagesName*/ ctx[8].page,
    				$$slots: { default: [create_default_slot$7] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(card.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(card, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const card_changes = {};
    			if (dirty & /*pages*/ 2) card_changes.title = /*pagesName*/ ctx[8].page;

    			if (dirty & /*$$scope, layoutJson, wsPush, pages*/ 8199) {
    				card_changes.$$scope = { dirty, ctx };
    			}

    			card.$set(card_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(card.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(card.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(card, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block$5.name,
    		type: "each",
    		source: "(17:2) {#each pages as pagesName, i}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$b(ctx) {
    	let div;
    	let t;
    	let current;
    	let if_block = /*layoutJson*/ ctx[0] === [] && create_if_block_4(ctx);
    	let each_value = /*pages*/ ctx[1];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block$5(get_each_context$5(ctx, each_value, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			div = element("div");
    			if (if_block) if_block.c();
    			t = space();

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			attr_dev(div, "class", "grd-3col1");
    			add_location(div, file$b, 12, 0, 323);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			if (if_block) if_block.m(div, null);
    			append_dev(div, t);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(div, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*layoutJson*/ ctx[0] === []) {
    				if (if_block) {
    					if (dirty & /*layoutJson*/ 1) {
    						transition_in(if_block, 1);
    					}
    				} else {
    					if_block = create_if_block_4(ctx);
    					if_block.c();
    					transition_in(if_block, 1);
    					if_block.m(div, t);
    				}
    			} else if (if_block) {
    				group_outros();

    				transition_out(if_block, 1, 1, () => {
    					if_block = null;
    				});

    				check_outros();
    			}

    			if (dirty & /*pages, layoutJson, wsPush*/ 7) {
    				each_value = /*pages*/ ctx[1];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context$5(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block$5(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(div, null);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(if_block);

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(if_block);
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			if (if_block) if_block.d();
    			destroy_each(each_blocks, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$b.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$b($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Dashboard', slots, []);
    	let { layoutJson } = $$props;
    	let { pages } = $$props;

    	let { wsPush = (ws, topic, status) => {
    		
    	} } = $$props;

    	const writable_props = ['layoutJson', 'pages', 'wsPush'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Dashboard> was created with unknown prop '${key}'`);
    	});

    	const func = (ws, topic, status) => wsPush(ws, topic, status);

    	function input_value_binding(value, widget) {
    		if ($$self.$$.not_equal(widget.status, value)) {
    			widget.status = value;
    			$$invalidate(0, layoutJson);
    		}
    	}

    	const func_1 = (ws, topic, status) => wsPush(ws, topic, status);

    	function toggle_value_binding(value, widget) {
    		if ($$self.$$.not_equal(widget.status, value)) {
    			widget.status = value;
    			$$invalidate(0, layoutJson);
    		}
    	}

    	function anydata_value_binding(value, widget) {
    		if ($$self.$$.not_equal(widget.status, value)) {
    			widget.status = value;
    			$$invalidate(0, layoutJson);
    		}
    	}

    	$$self.$$set = $$props => {
    		if ('layoutJson' in $$props) $$invalidate(0, layoutJson = $$props.layoutJson);
    		if ('pages' in $$props) $$invalidate(1, pages = $$props.pages);
    		if ('wsPush' in $$props) $$invalidate(2, wsPush = $$props.wsPush);
    	};

    	$$self.$capture_state = () => ({
    		Card,
    		Input,
    		Toggle,
    		Anydata,
    		layoutJson,
    		pages,
    		wsPush
    	});

    	$$self.$inject_state = $$props => {
    		if ('layoutJson' in $$props) $$invalidate(0, layoutJson = $$props.layoutJson);
    		if ('pages' in $$props) $$invalidate(1, pages = $$props.pages);
    		if ('wsPush' in $$props) $$invalidate(2, wsPush = $$props.wsPush);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		layoutJson,
    		pages,
    		wsPush,
    		func,
    		input_value_binding,
    		func_1,
    		toggle_value_binding,
    		anydata_value_binding
    	];
    }

    class Dashboard extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$b, create_fragment$b, safe_not_equal, { layoutJson: 0, pages: 1, wsPush: 2 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Dashboard",
    			options,
    			id: create_fragment$b.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*layoutJson*/ ctx[0] === undefined && !('layoutJson' in props)) {
    			console.warn("<Dashboard> was created without expected prop 'layoutJson'");
    		}

    		if (/*pages*/ ctx[1] === undefined && !('pages' in props)) {
    			console.warn("<Dashboard> was created without expected prop 'pages'");
    		}
    	}

    	get layoutJson() {
    		throw new Error("<Dashboard>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set layoutJson(value) {
    		throw new Error("<Dashboard>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get pages() {
    		throw new Error("<Dashboard>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set pages(value) {
    		throw new Error("<Dashboard>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get wsPush() {
    		throw new Error("<Dashboard>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set wsPush(value) {
    		throw new Error("<Dashboard>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\svg\Cross.svelte generated by Svelte v3.46.4 */

    const file$a = "src\\svg\\Cross.svelte";

    function create_fragment$a(ctx) {
    	let svg;
    	let line0;
    	let line1;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			svg = svg_element("svg");
    			line0 = svg_element("line");
    			line1 = svg_element("line");
    			attr_dev(line0, "x1", "18");
    			attr_dev(line0, "y1", "6");
    			attr_dev(line0, "x2", "6");
    			attr_dev(line0, "y2", "18");
    			add_location(line0, file$a, 5, 188, 261);
    			attr_dev(line1, "x1", "6");
    			attr_dev(line1, "y1", "6");
    			attr_dev(line1, "x2", "18");
    			attr_dev(line1, "y2", "18");
    			add_location(line1, file$a, 5, 227, 300);
    			attr_dev(svg, "class", "h-6 w-6 text-red-400 cursor-pointer");
    			attr_dev(svg, "viewBox", "0 0 24 24");
    			attr_dev(svg, "fill", "none");
    			attr_dev(svg, "stroke", "currentColor");
    			attr_dev(svg, "stroke-width", "2");
    			attr_dev(svg, "stroke-linecap", "round");
    			attr_dev(svg, "stroke-linejoin", "round");
    			add_location(svg, file$a, 5, 0, 73);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, svg, anchor);
    			append_dev(svg, line0);
    			append_dev(svg, line1);

    			if (!mounted) {
    				dispose = listen_dev(
    					svg,
    					"click",
    					function () {
    						if (is_function(/*click*/ ctx[1](/*i*/ ctx[0]))) /*click*/ ctx[1](/*i*/ ctx[0]).apply(this, arguments);
    					},
    					false,
    					false,
    					false
    				);

    				mounted = true;
    			}
    		},
    		p: function update(new_ctx, [dirty]) {
    			ctx = new_ctx;
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(svg);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$a.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$a($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Cross', slots, []);
    	let { i } = $$props;

    	let { click = i => {
    		
    	} } = $$props;

    	const writable_props = ['i', 'click'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Cross> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('i' in $$props) $$invalidate(0, i = $$props.i);
    		if ('click' in $$props) $$invalidate(1, click = $$props.click);
    	};

    	$$self.$capture_state = () => ({ i, click });

    	$$self.$inject_state = $$props => {
    		if ('i' in $$props) $$invalidate(0, i = $$props.i);
    		if ('click' in $$props) $$invalidate(1, click = $$props.click);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [i, click];
    }

    class Cross extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$a, create_fragment$a, safe_not_equal, { i: 0, click: 1 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Cross",
    			options,
    			id: create_fragment$a.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*i*/ ctx[0] === undefined && !('i' in props)) {
    			console.warn("<Cross> was created without expected prop 'i'");
    		}
    	}

    	get i() {
    		throw new Error("<Cross>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set i(value) {
    		throw new Error("<Cross>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get click() {
    		throw new Error("<Cross>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set click(value) {
    		throw new Error("<Cross>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\svg\Open.svelte generated by Svelte v3.46.4 */

    const file$9 = "src\\svg\\Open.svelte";

    function create_fragment$9(ctx) {
    	let svg;
    	let path;
    	let circle0;
    	let circle1;
    	let circle2;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			svg = svg_element("svg");
    			path = svg_element("path");
    			circle0 = svg_element("circle");
    			circle1 = svg_element("circle");
    			circle2 = svg_element("circle");
    			attr_dev(path, "stroke", "none");
    			attr_dev(path, "d", "M0 0h24v24H0z");
    			add_location(path, file$9, 4, 212, 267);
    			attr_dev(circle0, "cx", "5");
    			attr_dev(circle0, "cy", "12");
    			attr_dev(circle0, "r", "1");
    			add_location(circle0, file$9, 4, 253, 308);
    			attr_dev(circle1, "cx", "12");
    			attr_dev(circle1, "cy", "12");
    			attr_dev(circle1, "r", "1");
    			add_location(circle1, file$9, 4, 285, 340);
    			attr_dev(circle2, "cx", "19");
    			attr_dev(circle2, "cy", "12");
    			attr_dev(circle2, "r", "1");
    			add_location(circle2, file$9, 4, 318, 373);
    			attr_dev(svg, "class", "h-6 w-6 text-green-400 cursor-pointer");
    			attr_dev(svg, "width", "24");
    			attr_dev(svg, "height", "24");
    			attr_dev(svg, "viewBox", "0 0 24 24");
    			attr_dev(svg, "stroke-width", "2");
    			attr_dev(svg, "stroke", "currentColor");
    			attr_dev(svg, "fill", "none");
    			attr_dev(svg, "stroke-linecap", "round");
    			attr_dev(svg, "stroke-linejoin", "round");
    			add_location(svg, file$9, 4, 0, 55);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, svg, anchor);
    			append_dev(svg, path);
    			append_dev(svg, circle0);
    			append_dev(svg, circle1);
    			append_dev(svg, circle2);

    			if (!mounted) {
    				dispose = listen_dev(
    					svg,
    					"click",
    					function () {
    						if (is_function(/*click*/ ctx[0]())) /*click*/ ctx[0]().apply(this, arguments);
    					},
    					false,
    					false,
    					false
    				);

    				mounted = true;
    			}
    		},
    		p: function update(new_ctx, [dirty]) {
    			ctx = new_ctx;
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(svg);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$9.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$9($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Open', slots, []);

    	let { click = () => {
    		
    	} } = $$props;

    	const writable_props = ['click'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Open> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('click' in $$props) $$invalidate(0, click = $$props.click);
    	};

    	$$self.$capture_state = () => ({ click });

    	$$self.$inject_state = $$props => {
    		if ('click' in $$props) $$invalidate(0, click = $$props.click);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [click];
    }

    class Open extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$9, create_fragment$9, safe_not_equal, { click: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Open",
    			options,
    			id: create_fragment$9.name
    		});
    	}

    	get click() {
    		throw new Error("<Open>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set click(value) {
    		throw new Error("<Open>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\pages\Config.svelte generated by Svelte v3.46.4 */

    const { Object: Object_1$2, console: console_1$3 } = globals;
    const file$8 = "src\\pages\\Config.svelte";

    function get_each_context$4(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[19] = list[i];
    	child_ctx[20] = list;
    	child_ctx[21] = i;
    	return child_ctx;
    }

    function get_each_context_1(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[22] = list[i][0];
    	child_ctx[23] = list[i][1];
    	child_ctx[24] = list;
    	child_ctx[25] = i;
    	return child_ctx;
    }

    function get_each_context_2(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[26] = list[i];
    	return child_ctx;
    }

    function get_each_context_3(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[29] = list[i];
    	return child_ctx;
    }

    // (50:10) {#if item.header}
    function create_if_block_3(ctx) {
    	let optgroup;
    	let optgroup_label_value;

    	const block = {
    		c: function create() {
    			optgroup = element("optgroup");
    			attr_dev(optgroup, "label", optgroup_label_value = /*item*/ ctx[29].header);
    			add_location(optgroup, file$8, 50, 12, 1426);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, optgroup, anchor);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty[0] & /*itemsJson*/ 4 && optgroup_label_value !== (optgroup_label_value = /*item*/ ctx[29].header)) {
    				attr_dev(optgroup, "label", optgroup_label_value);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(optgroup);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_3.name,
    		type: "if",
    		source: "(50:10) {#if item.header}",
    		ctx
    	});

    	return block;
    }

    // (53:10) {#if !item.header}
    function create_if_block_2(ctx) {
    	let option;
    	let t0_value = /*item*/ ctx[29].name + "";
    	let t0;
    	let t1;
    	let option_value_value;

    	const block = {
    		c: function create() {
    			option = element("option");
    			t0 = text(t0_value);
    			t1 = space();
    			option.__value = option_value_value = /*item*/ ctx[29].num;
    			option.value = option.__value;
    			add_location(option, file$8, 53, 12, 1519);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, option, anchor);
    			append_dev(option, t0);
    			append_dev(option, t1);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty[0] & /*itemsJson*/ 4 && t0_value !== (t0_value = /*item*/ ctx[29].name + "")) set_data_dev(t0, t0_value);

    			if (dirty[0] & /*itemsJson*/ 4 && option_value_value !== (option_value_value = /*item*/ ctx[29].num)) {
    				prop_dev(option, "__value", option_value_value);
    				option.value = option.__value;
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(option);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_2.name,
    		type: "if",
    		source: "(53:10) {#if !item.header}",
    		ctx
    	});

    	return block;
    }

    // (49:8) {#each itemsJson as item}
    function create_each_block_3(ctx) {
    	let if_block0_anchor;
    	let if_block1_anchor;
    	let if_block0 = /*item*/ ctx[29].header && create_if_block_3(ctx);
    	let if_block1 = !/*item*/ ctx[29].header && create_if_block_2(ctx);

    	const block = {
    		c: function create() {
    			if (if_block0) if_block0.c();
    			if_block0_anchor = empty();
    			if (if_block1) if_block1.c();
    			if_block1_anchor = empty();
    		},
    		m: function mount(target, anchor) {
    			if (if_block0) if_block0.m(target, anchor);
    			insert_dev(target, if_block0_anchor, anchor);
    			if (if_block1) if_block1.m(target, anchor);
    			insert_dev(target, if_block1_anchor, anchor);
    		},
    		p: function update(ctx, dirty) {
    			if (/*item*/ ctx[29].header) {
    				if (if_block0) {
    					if_block0.p(ctx, dirty);
    				} else {
    					if_block0 = create_if_block_3(ctx);
    					if_block0.c();
    					if_block0.m(if_block0_anchor.parentNode, if_block0_anchor);
    				}
    			} else if (if_block0) {
    				if_block0.d(1);
    				if_block0 = null;
    			}

    			if (!/*item*/ ctx[29].header) {
    				if (if_block1) {
    					if_block1.p(ctx, dirty);
    				} else {
    					if_block1 = create_if_block_2(ctx);
    					if_block1.c();
    					if_block1.m(if_block1_anchor.parentNode, if_block1_anchor);
    				}
    			} else if (if_block1) {
    				if_block1.d(1);
    				if_block1 = null;
    			}
    		},
    		d: function destroy(detaching) {
    			if (if_block0) if_block0.d(detaching);
    			if (detaching) detach_dev(if_block0_anchor);
    			if (if_block1) if_block1.d(detaching);
    			if (detaching) detach_dev(if_block1_anchor);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block_3.name,
    		type: "each",
    		source: "(49:8) {#each itemsJson as item}",
    		ctx
    	});

    	return block;
    }

    // (81:16) {#each widgetsJson as select}
    function create_each_block_2(ctx) {
    	let option;
    	let t0_value = /*select*/ ctx[26].label + "";
    	let t0;
    	let t1;
    	let option_value_value;

    	const block = {
    		c: function create() {
    			option = element("option");
    			t0 = text(t0_value);
    			t1 = space();
    			option.__value = option_value_value = /*select*/ ctx[26].name;
    			option.value = option.__value;
    			add_location(option, file$8, 81, 18, 2614);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, option, anchor);
    			append_dev(option, t0);
    			append_dev(option, t1);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty[0] & /*widgetsJson*/ 2 && t0_value !== (t0_value = /*select*/ ctx[26].label + "")) set_data_dev(t0, t0_value);

    			if (dirty[0] & /*widgetsJson*/ 2 && option_value_value !== (option_value_value = /*select*/ ctx[26].name)) {
    				prop_dev(option, "__value", option_value_value);
    				option.value = option.__value;
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(option);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block_2.name,
    		type: "each",
    		source: "(81:16) {#each widgetsJson as select}",
    		ctx
    	});

    	return block;
    }

    // (92:10) {#if !hideAllSubParams}
    function create_if_block$3(ctx) {
    	let t;
    	let each_value_1 = Object.entries(/*element*/ ctx[19]);
    	validate_each_argument(each_value_1);
    	let each_blocks = [];

    	for (let i = 0; i < each_value_1.length; i += 1) {
    		each_blocks[i] = create_each_block_1(get_each_context_1(ctx, each_value_1, i));
    	}

    	const block = {
    		c: function create() {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			t = space();
    		},
    		m: function mount(target, anchor) {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(target, anchor);
    			}

    			insert_dev(target, t, anchor);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty[0] & /*configJson*/ 1) {
    				each_value_1 = Object.entries(/*element*/ ctx[19]);
    				validate_each_argument(each_value_1);
    				let i;

    				for (i = 0; i < each_value_1.length; i += 1) {
    					const child_ctx = get_each_context_1(ctx, each_value_1, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    					} else {
    						each_blocks[i] = create_each_block_1(child_ctx);
    						each_blocks[i].c();
    						each_blocks[i].m(t.parentNode, t);
    					}
    				}

    				for (; i < each_blocks.length; i += 1) {
    					each_blocks[i].d(1);
    				}

    				each_blocks.length = each_value_1.length;
    			}
    		},
    		d: function destroy(detaching) {
    			destroy_each(each_blocks, detaching);
    			if (detaching) detach_dev(t);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$3.name,
    		type: "if",
    		source: "(92:10) {#if !hideAllSubParams}",
    		ctx
    	});

    	return block;
    }

    // (94:14) {#if key != "type" && key != "subtype" && key != "id" && key != "widget" && key != "page" && key != "descr"}
    function create_if_block_1(ctx) {
    	let tr;
    	let td0;
    	let t0;
    	let td1;
    	let t1;
    	let td2;
    	let t2;
    	let td3;
    	let p;
    	let t3_value = /*key*/ ctx[22] + "";
    	let t3;
    	let t4;
    	let td4;
    	let input;
    	let mounted;
    	let dispose;

    	function input_input_handler() {
    		/*input_input_handler*/ ctx[16].call(input, /*key*/ ctx[22], /*each_value*/ ctx[20], /*i*/ ctx[21]);
    	}

    	const block = {
    		c: function create() {
    			tr = element("tr");
    			td0 = element("td");
    			t0 = space();
    			td1 = element("td");
    			t1 = space();
    			td2 = element("td");
    			t2 = space();
    			td3 = element("td");
    			p = element("p");
    			t3 = text(t3_value);
    			t4 = space();
    			td4 = element("td");
    			input = element("input");
    			add_location(td0, file$8, 95, 18, 3494);
    			add_location(td1, file$8, 96, 18, 3520);
    			add_location(td2, file$8, 97, 18, 3546);
    			attr_dev(p, "class", "txt-ita");
    			add_location(p, file$8, 99, 20, 3628);
    			attr_dev(td3, "class", "tbl-bdy-sm text-right");
    			add_location(td3, file$8, 98, 18, 3572);
    			attr_dev(input, "class", "ipt-sm w-full");
    			attr_dev(input, "type", "text");
    			add_location(input, file$8, 102, 20, 3758);
    			attr_dev(td4, "class", "tbl-bdy-sm text-center");
    			add_location(td4, file$8, 101, 18, 3701);
    			attr_dev(tr, "class", "txt-sz txt-pad");
    			add_location(tr, file$8, 94, 16, 3447);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, tr, anchor);
    			append_dev(tr, td0);
    			append_dev(tr, t0);
    			append_dev(tr, td1);
    			append_dev(tr, t1);
    			append_dev(tr, td2);
    			append_dev(tr, t2);
    			append_dev(tr, td3);
    			append_dev(td3, p);
    			append_dev(p, t3);
    			append_dev(tr, t4);
    			append_dev(tr, td4);
    			append_dev(td4, input);
    			set_input_value(input, /*element*/ ctx[19][/*key*/ ctx[22]]);

    			if (!mounted) {
    				dispose = listen_dev(input, "input", input_input_handler);
    				mounted = true;
    			}
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    			if (dirty[0] & /*configJson*/ 1 && t3_value !== (t3_value = /*key*/ ctx[22] + "")) set_data_dev(t3, t3_value);

    			if (dirty[0] & /*configJson, widgetsJson*/ 3 && input.value !== /*element*/ ctx[19][/*key*/ ctx[22]]) {
    				set_input_value(input, /*element*/ ctx[19][/*key*/ ctx[22]]);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(tr);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_1.name,
    		type: "if",
    		source: "(94:14) {#if key != \\\"type\\\" && key != \\\"subtype\\\" && key != \\\"id\\\" && key != \\\"widget\\\" && key != \\\"page\\\" && key != \\\"descr\\\"}",
    		ctx
    	});

    	return block;
    }

    // (93:12) {#each Object.entries(element) as [key, param]}
    function create_each_block_1(ctx) {
    	let if_block_anchor;
    	let if_block = /*key*/ ctx[22] != "type" && /*key*/ ctx[22] != "subtype" && /*key*/ ctx[22] != "id" && /*key*/ ctx[22] != "widget" && /*key*/ ctx[22] != "page" && /*key*/ ctx[22] != "descr" && create_if_block_1(ctx);

    	const block = {
    		c: function create() {
    			if (if_block) if_block.c();
    			if_block_anchor = empty();
    		},
    		m: function mount(target, anchor) {
    			if (if_block) if_block.m(target, anchor);
    			insert_dev(target, if_block_anchor, anchor);
    		},
    		p: function update(ctx, dirty) {
    			if (/*key*/ ctx[22] != "type" && /*key*/ ctx[22] != "subtype" && /*key*/ ctx[22] != "id" && /*key*/ ctx[22] != "widget" && /*key*/ ctx[22] != "page" && /*key*/ ctx[22] != "descr") {
    				if (if_block) {
    					if_block.p(ctx, dirty);
    				} else {
    					if_block = create_if_block_1(ctx);
    					if_block.c();
    					if_block.m(if_block_anchor.parentNode, if_block_anchor);
    				}
    			} else if (if_block) {
    				if_block.d(1);
    				if_block = null;
    			}
    		},
    		d: function destroy(detaching) {
    			if (if_block) if_block.d(detaching);
    			if (detaching) detach_dev(if_block_anchor);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block_1.name,
    		type: "each",
    		source: "(93:12) {#each Object.entries(element) as [key, param]}",
    		ctx
    	});

    	return block;
    }

    // (75:8) {#each configJson as element, i}
    function create_each_block$4(ctx) {
    	let tr;
    	let td0;
    	let t0_value = /*element*/ ctx[19].subtype + "";
    	let t0;
    	let t1;
    	let td1;
    	let input0;
    	let t2;
    	let td2;
    	let select;
    	let t3;
    	let td3;
    	let input1;
    	let t4;
    	let td4;
    	let input2;
    	let t5;
    	let td5;
    	let openicon;
    	let t6;
    	let td6;
    	let crossicon;
    	let t7;
    	let if_block_anchor;
    	let current;
    	let mounted;
    	let dispose;

    	function input0_input_handler() {
    		/*input0_input_handler*/ ctx[10].call(input0, /*each_value*/ ctx[20], /*i*/ ctx[21]);
    	}

    	let each_value_2 = /*widgetsJson*/ ctx[1];
    	validate_each_argument(each_value_2);
    	let each_blocks = [];

    	for (let i = 0; i < each_value_2.length; i += 1) {
    		each_blocks[i] = create_each_block_2(get_each_context_2(ctx, each_value_2, i));
    	}

    	function select_change_handler() {
    		/*select_change_handler*/ ctx[11].call(select, /*each_value*/ ctx[20], /*i*/ ctx[21]);
    	}

    	function input1_input_handler() {
    		/*input1_input_handler*/ ctx[12].call(input1, /*each_value*/ ctx[20], /*i*/ ctx[21]);
    	}

    	function input2_input_handler() {
    		/*input2_input_handler*/ ctx[13].call(input2, /*each_value*/ ctx[20], /*i*/ ctx[21]);
    	}

    	openicon = new Open({
    			props: { click: /*func*/ ctx[14] },
    			$$inline: true
    		});

    	crossicon = new Cross({
    			props: {
    				i: /*i*/ ctx[21],
    				click: /*func_1*/ ctx[15]
    			},
    			$$inline: true
    		});

    	let if_block = !/*hideAllSubParams*/ ctx[5] && create_if_block$3(ctx);

    	const block = {
    		c: function create() {
    			tr = element("tr");
    			td0 = element("td");
    			t0 = text(t0_value);
    			t1 = space();
    			td1 = element("td");
    			input0 = element("input");
    			t2 = space();
    			td2 = element("td");
    			select = element("select");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			t3 = space();
    			td3 = element("td");
    			input1 = element("input");
    			t4 = space();
    			td4 = element("td");
    			input2 = element("input");
    			t5 = space();
    			td5 = element("td");
    			create_component(openicon.$$.fragment);
    			t6 = space();
    			td6 = element("td");
    			create_component(crossicon.$$.fragment);
    			t7 = space();
    			if (if_block) if_block.c();
    			if_block_anchor = empty();
    			attr_dev(td0, "class", "tbl-bdy-lg");
    			add_location(td0, file$8, 76, 12, 2282);
    			attr_dev(input0, "class", "ipt-lg w-full");
    			attr_dev(input0, "type", "text");
    			add_location(input0, file$8, 77, 35, 2364);
    			attr_dev(td1, "class", "tbl-bdy-lg");
    			add_location(td1, file$8, 77, 12, 2341);
    			attr_dev(select, "class", "ipt-lg w-full");
    			if (/*element*/ ctx[19].widget === void 0) add_render_callback(select_change_handler);
    			add_location(select, file$8, 79, 15, 2489);
    			attr_dev(td2, "class", "tbl-bdy-lg");
    			add_location(td2, file$8, 78, 12, 2450);
    			attr_dev(input1, "class", "ipt-lg w-full");
    			attr_dev(input1, "type", "text");
    			add_location(input1, file$8, 86, 35, 2799);
    			attr_dev(td3, "class", "tbl-bdy-lg");
    			add_location(td3, file$8, 86, 12, 2776);
    			attr_dev(input2, "class", "ipt-lg w-full");
    			attr_dev(input2, "type", "text");
    			add_location(input2, file$8, 87, 35, 2910);
    			attr_dev(td4, "class", "tbl-bdy-lg");
    			add_location(td4, file$8, 87, 12, 2887);
    			attr_dev(td5, "class", "tbl-bdy-lg");
    			add_location(td5, file$8, 88, 12, 2999);
    			attr_dev(td6, "class", "tbl-bdy-lg");
    			add_location(td6, file$8, 89, 12, 3106);
    			attr_dev(tr, "class", "txt-sz txt-pad align-middle");
    			add_location(tr, file$8, 75, 10, 2228);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, tr, anchor);
    			append_dev(tr, td0);
    			append_dev(td0, t0);
    			append_dev(tr, t1);
    			append_dev(tr, td1);
    			append_dev(td1, input0);
    			set_input_value(input0, /*element*/ ctx[19].id);
    			append_dev(tr, t2);
    			append_dev(tr, td2);
    			append_dev(td2, select);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(select, null);
    			}

    			select_option(select, /*element*/ ctx[19].widget);
    			append_dev(tr, t3);
    			append_dev(tr, td3);
    			append_dev(td3, input1);
    			set_input_value(input1, /*element*/ ctx[19].page);
    			append_dev(tr, t4);
    			append_dev(tr, td4);
    			append_dev(td4, input2);
    			set_input_value(input2, /*element*/ ctx[19].descr);
    			append_dev(tr, t5);
    			append_dev(tr, td5);
    			mount_component(openicon, td5, null);
    			append_dev(tr, t6);
    			append_dev(tr, td6);
    			mount_component(crossicon, td6, null);
    			insert_dev(target, t7, anchor);
    			if (if_block) if_block.m(target, anchor);
    			insert_dev(target, if_block_anchor, anchor);
    			current = true;

    			if (!mounted) {
    				dispose = [
    					listen_dev(input0, "input", input0_input_handler),
    					listen_dev(select, "change", select_change_handler),
    					listen_dev(input1, "input", input1_input_handler),
    					listen_dev(input2, "input", input2_input_handler)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    			if ((!current || dirty[0] & /*configJson*/ 1) && t0_value !== (t0_value = /*element*/ ctx[19].subtype + "")) set_data_dev(t0, t0_value);

    			if (dirty[0] & /*configJson, widgetsJson*/ 3 && input0.value !== /*element*/ ctx[19].id) {
    				set_input_value(input0, /*element*/ ctx[19].id);
    			}

    			if (dirty[0] & /*widgetsJson*/ 2) {
    				each_value_2 = /*widgetsJson*/ ctx[1];
    				validate_each_argument(each_value_2);
    				let i;

    				for (i = 0; i < each_value_2.length; i += 1) {
    					const child_ctx = get_each_context_2(ctx, each_value_2, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    					} else {
    						each_blocks[i] = create_each_block_2(child_ctx);
    						each_blocks[i].c();
    						each_blocks[i].m(select, null);
    					}
    				}

    				for (; i < each_blocks.length; i += 1) {
    					each_blocks[i].d(1);
    				}

    				each_blocks.length = each_value_2.length;
    			}

    			if (dirty[0] & /*configJson, widgetsJson*/ 3) {
    				select_option(select, /*element*/ ctx[19].widget);
    			}

    			if (dirty[0] & /*configJson, widgetsJson*/ 3 && input1.value !== /*element*/ ctx[19].page) {
    				set_input_value(input1, /*element*/ ctx[19].page);
    			}

    			if (dirty[0] & /*configJson, widgetsJson*/ 3 && input2.value !== /*element*/ ctx[19].descr) {
    				set_input_value(input2, /*element*/ ctx[19].descr);
    			}

    			const openicon_changes = {};
    			if (dirty[0] & /*hideAllSubParams*/ 32) openicon_changes.click = /*func*/ ctx[14];
    			openicon.$set(openicon_changes);

    			if (!/*hideAllSubParams*/ ctx[5]) {
    				if (if_block) {
    					if_block.p(ctx, dirty);
    				} else {
    					if_block = create_if_block$3(ctx);
    					if_block.c();
    					if_block.m(if_block_anchor.parentNode, if_block_anchor);
    				}
    			} else if (if_block) {
    				if_block.d(1);
    				if_block = null;
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(openicon.$$.fragment, local);
    			transition_in(crossicon.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(openicon.$$.fragment, local);
    			transition_out(crossicon.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(tr);
    			destroy_each(each_blocks, detaching);
    			destroy_component(openicon);
    			destroy_component(crossicon);
    			if (detaching) detach_dev(t7);
    			if (if_block) if_block.d(detaching);
    			if (detaching) detach_dev(if_block_anchor);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block$4.name,
    		type: "each",
    		source: "(75:8) {#each configJson as element, i}",
    		ctx
    	});

    	return block;
    }

    // (46:2) <Card>
    function create_default_slot$6(ctx) {
    	let div;
    	let select0;
    	let t0;
    	let select1;
    	let option;
    	let t2;
    	let table;
    	let thead;
    	let tr;
    	let th0;
    	let t4;
    	let th1;
    	let t6;
    	let th2;
    	let t8;
    	let th3;
    	let t10;
    	let th4;
    	let t12;
    	let th5;
    	let t13;
    	let th6;
    	let t14;
    	let tbody;
    	let t15;
    	let button;
    	let current;
    	let mounted;
    	let dispose;
    	let each_value_3 = /*itemsJson*/ ctx[2];
    	validate_each_argument(each_value_3);
    	let each_blocks_1 = [];

    	for (let i = 0; i < each_value_3.length; i += 1) {
    		each_blocks_1[i] = create_each_block_3(get_each_context_3(ctx, each_value_3, i));
    	}

    	let each_value = /*configJson*/ ctx[0];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block$4(get_each_context$4(ctx, each_value, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			div = element("div");
    			select0 = element("select");

    			for (let i = 0; i < each_blocks_1.length; i += 1) {
    				each_blocks_1[i].c();
    			}

    			t0 = space();
    			select1 = element("select");
    			option = element("option");
    			option.textContent = `${"Выберите пресет"}`;
    			t2 = space();
    			table = element("table");
    			thead = element("thead");
    			tr = element("tr");
    			th0 = element("th");
    			th0.textContent = "Тип";
    			t4 = space();
    			th1 = element("th");
    			th1.textContent = "Id";
    			t6 = space();
    			th2 = element("th");
    			th2.textContent = "Виджет";
    			t8 = space();
    			th3 = element("th");
    			th3.textContent = "Вкладка";
    			t10 = space();
    			th4 = element("th");
    			th4.textContent = "Название";
    			t12 = space();
    			th5 = element("th");
    			t13 = space();
    			th6 = element("th");
    			t14 = space();
    			tbody = element("tbody");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			t15 = space();
    			button = element("button");
    			button.textContent = `${"Сохранить"}`;
    			attr_dev(select0, "class", "slct-lg");
    			if (/*itemsJsonBind*/ ctx[4] === void 0) add_render_callback(() => /*select0_change_handler*/ ctx[8].call(select0));
    			add_location(select0, file$8, 47, 6, 1254);
    			option.__value = "Выберите пресет";
    			option.value = option.__value;
    			add_location(option, file$8, 59, 30, 1677);
    			attr_dev(select1, "class", "slct-lg");
    			add_location(select1, file$8, 59, 6, 1653);
    			attr_dev(div, "class", "grd-2col2");
    			add_location(div, file$8, 46, 4, 1223);
    			attr_dev(th0, "class", "tbl-hd");
    			add_location(th0, file$8, 64, 10, 1843);
    			attr_dev(th1, "class", "tbl-hd");
    			add_location(th1, file$8, 65, 10, 1882);
    			attr_dev(th2, "class", "tbl-hd");
    			add_location(th2, file$8, 66, 10, 1920);
    			attr_dev(th3, "class", "tbl-hd");
    			add_location(th3, file$8, 67, 10, 1962);
    			attr_dev(th4, "class", "tbl-hd");
    			add_location(th4, file$8, 68, 10, 2005);
    			attr_dev(th5, "class", "tbl-hd w-7");
    			add_location(th5, file$8, 69, 10, 2049);
    			attr_dev(th6, "class", "tbl-hd w-7");
    			add_location(th6, file$8, 70, 10, 2086);
    			attr_dev(tr, "class", "txt-sz txt-pad");
    			add_location(tr, file$8, 63, 8, 1804);
    			attr_dev(thead, "class", "bg-gray-100");
    			add_location(thead, file$8, 62, 6, 1767);
    			attr_dev(tbody, "class", "bg-white");
    			add_location(tbody, file$8, 73, 6, 2150);
    			attr_dev(table, "class", "tbl");
    			add_location(table, file$8, 61, 4, 1740);
    			attr_dev(button, "class", "btn-lg");
    			add_location(button, file$8, 112, 4, 4014);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			append_dev(div, select0);

    			for (let i = 0; i < each_blocks_1.length; i += 1) {
    				each_blocks_1[i].m(select0, null);
    			}

    			select_option(select0, /*itemsJsonBind*/ ctx[4]);
    			append_dev(div, t0);
    			append_dev(div, select1);
    			append_dev(select1, option);
    			insert_dev(target, t2, anchor);
    			insert_dev(target, table, anchor);
    			append_dev(table, thead);
    			append_dev(thead, tr);
    			append_dev(tr, th0);
    			append_dev(tr, t4);
    			append_dev(tr, th1);
    			append_dev(tr, t6);
    			append_dev(tr, th2);
    			append_dev(tr, t8);
    			append_dev(tr, th3);
    			append_dev(tr, t10);
    			append_dev(tr, th4);
    			append_dev(tr, t12);
    			append_dev(tr, th5);
    			append_dev(tr, t13);
    			append_dev(tr, th6);
    			append_dev(table, t14);
    			append_dev(table, tbody);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(tbody, null);
    			}

    			insert_dev(target, t15, anchor);
    			insert_dev(target, button, anchor);
    			current = true;

    			if (!mounted) {
    				dispose = [
    					listen_dev(select0, "change", /*select0_change_handler*/ ctx[8]),
    					listen_dev(select0, "change", /*change_handler*/ ctx[9], false, false, false),
    					listen_dev(button, "click", /*click_handler*/ ctx[17], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty[0] & /*itemsJson*/ 4) {
    				each_value_3 = /*itemsJson*/ ctx[2];
    				validate_each_argument(each_value_3);
    				let i;

    				for (i = 0; i < each_value_3.length; i += 1) {
    					const child_ctx = get_each_context_3(ctx, each_value_3, i);

    					if (each_blocks_1[i]) {
    						each_blocks_1[i].p(child_ctx, dirty);
    					} else {
    						each_blocks_1[i] = create_each_block_3(child_ctx);
    						each_blocks_1[i].c();
    						each_blocks_1[i].m(select0, null);
    					}
    				}

    				for (; i < each_blocks_1.length; i += 1) {
    					each_blocks_1[i].d(1);
    				}

    				each_blocks_1.length = each_value_3.length;
    			}

    			if (dirty[0] & /*itemsJsonBind, itemsJson*/ 20) {
    				select_option(select0, /*itemsJsonBind*/ ctx[4]);
    			}

    			if (dirty[0] & /*configJson, hideAllSubParams, deleteLineFromConfig, widgetsJson*/ 163) {
    				each_value = /*configJson*/ ctx[0];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context$4(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block$4(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(tbody, null);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

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
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			destroy_each(each_blocks_1, detaching);
    			if (detaching) detach_dev(t2);
    			if (detaching) detach_dev(table);
    			destroy_each(each_blocks, detaching);
    			if (detaching) detach_dev(t15);
    			if (detaching) detach_dev(button);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot$6.name,
    		type: "slot",
    		source: "(46:2) <Card>",
    		ctx
    	});

    	return block;
    }

    function create_fragment$8(ctx) {
    	let div;
    	let card;
    	let current;

    	card = new Card({
    			props: {
    				$$slots: { default: [create_default_slot$6] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			div = element("div");
    			create_component(card.$$.fragment);
    			attr_dev(div, "class", "grd-1col1");
    			add_location(div, file$8, 44, 0, 1184);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			mount_component(card, div, null);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const card_changes = {};

    			if (dirty[0] & /*saveConfig, configJson, hideAllSubParams, widgetsJson, itemsJsonBind, itemsJson*/ 63 | dirty[1] & /*$$scope*/ 2) {
    				card_changes.$$scope = { dirty, ctx };
    			}

    			card.$set(card_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(card.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(card.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			destroy_component(card);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$8.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$8($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Config', slots, []);
    	let { configJson } = $$props;
    	let { widgetsJson } = $$props;
    	let { itemsJson } = $$props;
    	let itemsJsonBind = 0;
    	let debug = true;

    	let { saveConfig = () => {
    		
    	} } = $$props;

    	let hideAllSubParams = true;

    	function elementsDropdownChange() {
    		for (let i = 0; i < itemsJson.length; i++) {
    			let item = Object.assign({}, itemsJson[i]);

    			if (itemsJsonBind === item.num) {
    				delete item.num;
    				delete item.name;
    				configJson.push(item);
    				$$invalidate(0, configJson);
    				$$invalidate(4, itemsJsonBind = 0);
    				if (debug) console.log("[i]", "item added");
    				break;
    			}
    		}
    	}

    	function deleteLineFromConfig(num) {
    		for (let i = 0; i < configJson.length; i++) {
    			if (num === i) {
    				configJson.splice(i, 1);
    				$$invalidate(0, configJson);
    				if (debug) console.log("[i]", "item " + num + " deleted from config");
    				break;
    			}
    		}
    	}

    	const writable_props = ['configJson', 'widgetsJson', 'itemsJson', 'saveConfig'];

    	Object_1$2.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console_1$3.warn(`<Config> was created with unknown prop '${key}'`);
    	});

    	function select0_change_handler() {
    		itemsJsonBind = select_value(this);
    		$$invalidate(4, itemsJsonBind);
    		$$invalidate(2, itemsJson);
    	}

    	const change_handler = () => elementsDropdownChange();

    	function input0_input_handler(each_value, i) {
    		each_value[i].id = this.value;
    		$$invalidate(0, configJson);
    		$$invalidate(1, widgetsJson);
    	}

    	function select_change_handler(each_value, i) {
    		each_value[i].widget = select_value(this);
    		$$invalidate(0, configJson);
    		$$invalidate(1, widgetsJson);
    	}

    	function input1_input_handler(each_value, i) {
    		each_value[i].page = this.value;
    		$$invalidate(0, configJson);
    		$$invalidate(1, widgetsJson);
    	}

    	function input2_input_handler(each_value, i) {
    		each_value[i].descr = this.value;
    		$$invalidate(0, configJson);
    		$$invalidate(1, widgetsJson);
    	}

    	const func = () => $$invalidate(5, hideAllSubParams = !hideAllSubParams);
    	const func_1 = i => deleteLineFromConfig(i);

    	function input_input_handler(key, each_value, i) {
    		each_value[i][key] = this.value;
    		$$invalidate(0, configJson);
    		$$invalidate(1, widgetsJson);
    	}

    	const click_handler = () => saveConfig();

    	$$self.$$set = $$props => {
    		if ('configJson' in $$props) $$invalidate(0, configJson = $$props.configJson);
    		if ('widgetsJson' in $$props) $$invalidate(1, widgetsJson = $$props.widgetsJson);
    		if ('itemsJson' in $$props) $$invalidate(2, itemsJson = $$props.itemsJson);
    		if ('saveConfig' in $$props) $$invalidate(3, saveConfig = $$props.saveConfig);
    	};

    	$$self.$capture_state = () => ({
    		Card,
    		CrossIcon: Cross,
    		OpenIcon: Open,
    		configJson,
    		widgetsJson,
    		itemsJson,
    		itemsJsonBind,
    		debug,
    		saveConfig,
    		hideAllSubParams,
    		elementsDropdownChange,
    		deleteLineFromConfig
    	});

    	$$self.$inject_state = $$props => {
    		if ('configJson' in $$props) $$invalidate(0, configJson = $$props.configJson);
    		if ('widgetsJson' in $$props) $$invalidate(1, widgetsJson = $$props.widgetsJson);
    		if ('itemsJson' in $$props) $$invalidate(2, itemsJson = $$props.itemsJson);
    		if ('itemsJsonBind' in $$props) $$invalidate(4, itemsJsonBind = $$props.itemsJsonBind);
    		if ('debug' in $$props) debug = $$props.debug;
    		if ('saveConfig' in $$props) $$invalidate(3, saveConfig = $$props.saveConfig);
    		if ('hideAllSubParams' in $$props) $$invalidate(5, hideAllSubParams = $$props.hideAllSubParams);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		configJson,
    		widgetsJson,
    		itemsJson,
    		saveConfig,
    		itemsJsonBind,
    		hideAllSubParams,
    		elementsDropdownChange,
    		deleteLineFromConfig,
    		select0_change_handler,
    		change_handler,
    		input0_input_handler,
    		select_change_handler,
    		input1_input_handler,
    		input2_input_handler,
    		func,
    		func_1,
    		input_input_handler,
    		click_handler
    	];
    }

    class Config extends SvelteComponentDev {
    	constructor(options) {
    		super(options);

    		init(
    			this,
    			options,
    			instance$8,
    			create_fragment$8,
    			safe_not_equal,
    			{
    				configJson: 0,
    				widgetsJson: 1,
    				itemsJson: 2,
    				saveConfig: 3
    			},
    			null,
    			[-1, -1]
    		);

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Config",
    			options,
    			id: create_fragment$8.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*configJson*/ ctx[0] === undefined && !('configJson' in props)) {
    			console_1$3.warn("<Config> was created without expected prop 'configJson'");
    		}

    		if (/*widgetsJson*/ ctx[1] === undefined && !('widgetsJson' in props)) {
    			console_1$3.warn("<Config> was created without expected prop 'widgetsJson'");
    		}

    		if (/*itemsJson*/ ctx[2] === undefined && !('itemsJson' in props)) {
    			console_1$3.warn("<Config> was created without expected prop 'itemsJson'");
    		}
    	}

    	get configJson() {
    		throw new Error("<Config>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set configJson(value) {
    		throw new Error("<Config>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get widgetsJson() {
    		throw new Error("<Config>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set widgetsJson(value) {
    		throw new Error("<Config>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get itemsJson() {
    		throw new Error("<Config>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set itemsJson(value) {
    		throw new Error("<Config>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get saveConfig() {
    		throw new Error("<Config>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set saveConfig(value) {
    		throw new Error("<Config>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\pages\Connection.svelte generated by Svelte v3.46.4 */

    const { Object: Object_1$1 } = globals;
    const file$7 = "src\\pages\\Connection.svelte";

    function get_each_context$3(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[17] = list[i][0];
    	child_ctx[18] = list[i][1];
    	return child_ctx;
    }

    // (43:10) {#each Object.entries(ssidJson) as [num, ssid]}
    function create_each_block$3(ctx) {
    	let option;
    	let t0_value = /*ssid*/ ctx[18] + "";
    	let t0;
    	let t1;
    	let option_value_value;

    	const block = {
    		c: function create() {
    			option = element("option");
    			t0 = text(t0_value);
    			t1 = space();
    			option.__value = option_value_value = /*ssid*/ ctx[18];
    			option.value = option.__value;
    			add_location(option, file$7, 43, 12, 1599);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, option, anchor);
    			append_dev(option, t0);
    			append_dev(option, t1);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*ssidJson*/ 2 && t0_value !== (t0_value = /*ssid*/ ctx[18] + "")) set_data_dev(t0, t0_value);

    			if (dirty & /*ssidJson*/ 2 && option_value_value !== (option_value_value = /*ssid*/ ctx[18])) {
    				prop_dev(option, "__value", option_value_value);
    				option.value = option.__value;
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(option);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block$3.name,
    		type: "each",
    		source: "(43:10) {#each Object.entries(ssidJson) as [num, ssid]}",
    		ctx
    	});

    	return block;
    }

    // (59:4) {#if settingsJson.pass_status === 1}
    function create_if_block$2(ctx) {
    	let div;
    	let alarm;
    	let current;

    	alarm = new Alarm({
    			props: { title: "Введен неправильный пароль" },
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			div = element("div");
    			create_component(alarm.$$.fragment);
    			attr_dev(div, "class", "grd-1col1");
    			add_location(div, file$7, 59, 6, 2071);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			mount_component(alarm, div, null);
    			current = true;
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(alarm.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(alarm.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			destroy_component(alarm);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$2.name,
    		type: "if",
    		source: "(59:4) {#if settingsJson.pass_status === 1}",
    		ctx
    	});

    	return block;
    }

    // (12:2) <Card title="Подключение к WiFi роутеру">
    function create_default_slot_1$3(ctx) {
    	let div2;
    	let div0;
    	let p0;
    	let t1;
    	let div1;
    	let input0;
    	let t2;
    	let div5;
    	let div3;
    	let p1;
    	let t4;
    	let div4;
    	let input1;
    	let t5;
    	let div8;
    	let div6;
    	let p2;
    	let t7;
    	let div7;
    	let input2;
    	let t8;
    	let div11;
    	let div9;
    	let p3;
    	let t10;
    	let div10;
    	let select;
    	let t11;
    	let div14;
    	let div12;
    	let p4;
    	let t13;
    	let div13;
    	let input3;
    	let t14;
    	let t15;
    	let button;
    	let current;
    	let mounted;
    	let dispose;
    	let each_value = Object.entries(/*ssidJson*/ ctx[1]);
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block$3(get_each_context$3(ctx, each_value, i));
    	}

    	let if_block = /*settingsJson*/ ctx[0].pass_status === 1 && create_if_block$2(ctx);

    	const block = {
    		c: function create() {
    			div2 = element("div");
    			div0 = element("div");
    			p0 = element("p");
    			p0.textContent = "Название устройства";
    			t1 = space();
    			div1 = element("div");
    			input0 = element("input");
    			t2 = space();
    			div5 = element("div");
    			div3 = element("div");
    			p1 = element("p");
    			p1.textContent = "Точка доступа";
    			t4 = space();
    			div4 = element("div");
    			input1 = element("input");
    			t5 = space();
    			div8 = element("div");
    			div6 = element("div");
    			p2 = element("p");
    			p2.textContent = "Пароль точки доступа";
    			t7 = space();
    			div7 = element("div");
    			input2 = element("input");
    			t8 = space();
    			div11 = element("div");
    			div9 = element("div");
    			p3 = element("p");
    			p3.textContent = "Название wifi сети";
    			t10 = space();
    			div10 = element("div");
    			select = element("select");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			t11 = space();
    			div14 = element("div");
    			div12 = element("div");
    			p4 = element("p");
    			p4.textContent = "Пароль";
    			t13 = space();
    			div13 = element("div");
    			input3 = element("input");
    			t14 = space();
    			if (if_block) if_block.c();
    			t15 = space();
    			button = element("button");
    			button.textContent = `${"Сохранить и перезагрузить"}`;
    			attr_dev(p0, "class", "wgt-dscr-stl");
    			add_location(p0, file$7, 14, 8, 401);
    			attr_dev(div0, "class", "wgt-dscr-w");
    			add_location(div0, file$7, 13, 6, 367);
    			attr_dev(input0, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input0, "type", "text");
    			add_location(input0, file$7, 17, 8, 499);
    			attr_dev(div1, "class", "wgt-w");
    			add_location(div1, file$7, 16, 6, 470);
    			attr_dev(div2, "class", "crd-itm-psn");
    			add_location(div2, file$7, 12, 4, 334);
    			attr_dev(p1, "class", "wgt-dscr-stl");
    			add_location(p1, file$7, 22, 8, 700);
    			attr_dev(div3, "class", "wgt-dscr-w");
    			add_location(div3, file$7, 21, 6, 666);
    			attr_dev(input1, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input1, "type", "text");
    			add_location(input1, file$7, 25, 8, 792);
    			attr_dev(div4, "class", "wgt-w");
    			add_location(div4, file$7, 24, 6, 763);
    			attr_dev(div5, "class", "crd-itm-psn");
    			add_location(div5, file$7, 20, 4, 633);
    			attr_dev(p2, "class", "wgt-dscr-stl");
    			add_location(p2, file$7, 30, 8, 995);
    			attr_dev(div6, "class", "wgt-dscr-w");
    			add_location(div6, file$7, 29, 6, 961);
    			attr_dev(input2, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input2, "type", "text");
    			add_location(input2, file$7, 33, 8, 1094);
    			attr_dev(div7, "class", "wgt-w");
    			add_location(div7, file$7, 32, 6, 1065);
    			attr_dev(div8, "class", "crd-itm-psn");
    			add_location(div8, file$7, 28, 4, 928);
    			attr_dev(p3, "class", "wgt-dscr-stl");
    			add_location(p3, file$7, 38, 8, 1297);
    			attr_dev(div9, "class", "wgt-dscr-w");
    			add_location(div9, file$7, 37, 6, 1263);
    			attr_dev(select, "class", "ipt-rnd text-left focus:border-indigo-500");
    			if (/*settingsJson*/ ctx[0].routerssid === void 0) add_render_callback(() => /*select_change_handler*/ ctx[7].call(select));
    			add_location(select, file$7, 41, 8, 1394);
    			attr_dev(div10, "class", "wgt-w");
    			add_location(div10, file$7, 40, 6, 1365);
    			attr_dev(div11, "class", "crd-itm-psn");
    			add_location(div11, file$7, 36, 4, 1230);
    			attr_dev(p4, "class", "wgt-dscr-stl");
    			add_location(p4, file$7, 52, 8, 1802);
    			attr_dev(div12, "class", "wgt-dscr-w");
    			add_location(div12, file$7, 51, 6, 1768);
    			attr_dev(input3, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input3, "type", "text");
    			add_location(input3, file$7, 55, 8, 1887);
    			attr_dev(div13, "class", "wgt-w");
    			add_location(div13, file$7, 54, 6, 1858);
    			attr_dev(div14, "class", "crd-itm-psn");
    			add_location(div14, file$7, 50, 4, 1735);
    			attr_dev(button, "class", "btn-lg");
    			add_location(button, file$7, 63, 4, 2179);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div0);
    			append_dev(div0, p0);
    			append_dev(div2, t1);
    			append_dev(div2, div1);
    			append_dev(div1, input0);
    			set_input_value(input0, /*settingsJson*/ ctx[0].name);
    			insert_dev(target, t2, anchor);
    			insert_dev(target, div5, anchor);
    			append_dev(div5, div3);
    			append_dev(div3, p1);
    			append_dev(div5, t4);
    			append_dev(div5, div4);
    			append_dev(div4, input1);
    			set_input_value(input1, /*settingsJson*/ ctx[0].apssid);
    			insert_dev(target, t5, anchor);
    			insert_dev(target, div8, anchor);
    			append_dev(div8, div6);
    			append_dev(div6, p2);
    			append_dev(div8, t7);
    			append_dev(div8, div7);
    			append_dev(div7, input2);
    			set_input_value(input2, /*settingsJson*/ ctx[0].appass);
    			insert_dev(target, t8, anchor);
    			insert_dev(target, div11, anchor);
    			append_dev(div11, div9);
    			append_dev(div9, p3);
    			append_dev(div11, t10);
    			append_dev(div11, div10);
    			append_dev(div10, select);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(select, null);
    			}

    			select_option(select, /*settingsJson*/ ctx[0].routerssid);
    			insert_dev(target, t11, anchor);
    			insert_dev(target, div14, anchor);
    			append_dev(div14, div12);
    			append_dev(div12, p4);
    			append_dev(div14, t13);
    			append_dev(div14, div13);
    			append_dev(div13, input3);
    			set_input_value(input3, /*settingsJson*/ ctx[0].routerpass);
    			insert_dev(target, t14, anchor);
    			if (if_block) if_block.m(target, anchor);
    			insert_dev(target, t15, anchor);
    			insert_dev(target, button, anchor);
    			current = true;

    			if (!mounted) {
    				dispose = [
    					listen_dev(input0, "input", /*input0_input_handler*/ ctx[4]),
    					listen_dev(input1, "input", /*input1_input_handler*/ ctx[5]),
    					listen_dev(input2, "input", /*input2_input_handler*/ ctx[6]),
    					listen_dev(select, "change", /*select_change_handler*/ ctx[7]),
    					listen_dev(select, "click", /*click_handler*/ ctx[8], false, false, false),
    					listen_dev(input3, "input", /*input3_input_handler*/ ctx[9]),
    					listen_dev(button, "click", /*click_handler_1*/ ctx[10], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input0.value !== /*settingsJson*/ ctx[0].name) {
    				set_input_value(input0, /*settingsJson*/ ctx[0].name);
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input1.value !== /*settingsJson*/ ctx[0].apssid) {
    				set_input_value(input1, /*settingsJson*/ ctx[0].apssid);
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input2.value !== /*settingsJson*/ ctx[0].appass) {
    				set_input_value(input2, /*settingsJson*/ ctx[0].appass);
    			}

    			if (dirty & /*Object, ssidJson*/ 2) {
    				each_value = Object.entries(/*ssidJson*/ ctx[1]);
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context$3(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    					} else {
    						each_blocks[i] = create_each_block$3(child_ctx);
    						each_blocks[i].c();
    						each_blocks[i].m(select, null);
    					}
    				}

    				for (; i < each_blocks.length; i += 1) {
    					each_blocks[i].d(1);
    				}

    				each_blocks.length = each_value.length;
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3) {
    				select_option(select, /*settingsJson*/ ctx[0].routerssid);
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input3.value !== /*settingsJson*/ ctx[0].routerpass) {
    				set_input_value(input3, /*settingsJson*/ ctx[0].routerpass);
    			}

    			if (/*settingsJson*/ ctx[0].pass_status === 1) {
    				if (if_block) {
    					if (dirty & /*settingsJson*/ 1) {
    						transition_in(if_block, 1);
    					}
    				} else {
    					if_block = create_if_block$2(ctx);
    					if_block.c();
    					transition_in(if_block, 1);
    					if_block.m(t15.parentNode, t15);
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
    			if (detaching) detach_dev(div2);
    			if (detaching) detach_dev(t2);
    			if (detaching) detach_dev(div5);
    			if (detaching) detach_dev(t5);
    			if (detaching) detach_dev(div8);
    			if (detaching) detach_dev(t8);
    			if (detaching) detach_dev(div11);
    			destroy_each(each_blocks, detaching);
    			if (detaching) detach_dev(t11);
    			if (detaching) detach_dev(div14);
    			if (detaching) detach_dev(t14);
    			if (if_block) if_block.d(detaching);
    			if (detaching) detach_dev(t15);
    			if (detaching) detach_dev(button);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_1$3.name,
    		type: "slot",
    		source: "(12:2) <Card title=\\\"Подключение к WiFi роутеру\\\">",
    		ctx
    	});

    	return block;
    }

    // (66:2) <Card title="Подключение к MQTT брокеру">
    function create_default_slot$5(ctx) {
    	let div2;
    	let div0;
    	let p0;
    	let t1;
    	let div1;
    	let input0;
    	let t2;
    	let div5;
    	let div3;
    	let p1;
    	let t4;
    	let div4;
    	let input1;
    	let t5;
    	let div8;
    	let div6;
    	let p2;
    	let t7;
    	let div7;
    	let input2;
    	let t8;
    	let div11;
    	let div9;
    	let p3;
    	let t10;
    	let div10;
    	let input3;
    	let t11;
    	let div14;
    	let div12;
    	let p4;
    	let t13;
    	let div13;
    	let input4;
    	let t14;
    	let button;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			div2 = element("div");
    			div0 = element("div");
    			p0 = element("p");
    			p0.textContent = "Название сервера";
    			t1 = space();
    			div1 = element("div");
    			input0 = element("input");
    			t2 = space();
    			div5 = element("div");
    			div3 = element("div");
    			p1 = element("p");
    			p1.textContent = "Порт";
    			t4 = space();
    			div4 = element("div");
    			input1 = element("input");
    			t5 = space();
    			div8 = element("div");
    			div6 = element("div");
    			p2 = element("p");
    			p2.textContent = "Префикс";
    			t7 = space();
    			div7 = element("div");
    			input2 = element("input");
    			t8 = space();
    			div11 = element("div");
    			div9 = element("div");
    			p3 = element("p");
    			p3.textContent = "Имя пользователя";
    			t10 = space();
    			div10 = element("div");
    			input3 = element("input");
    			t11 = space();
    			div14 = element("div");
    			div12 = element("div");
    			p4 = element("p");
    			p4.textContent = "Пароль";
    			t13 = space();
    			div13 = element("div");
    			input4 = element("input");
    			t14 = space();
    			button = element("button");
    			button.textContent = `${"Сохранить и проверить подключение"}`;
    			attr_dev(p0, "class", "wgt-dscr-stl");
    			add_location(p0, file$7, 68, 8, 2401);
    			attr_dev(div0, "class", "wgt-dscr-w");
    			add_location(div0, file$7, 67, 6, 2367);
    			attr_dev(input0, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input0, "type", "text");
    			add_location(input0, file$7, 71, 8, 2496);
    			attr_dev(div1, "class", "wgt-w");
    			add_location(div1, file$7, 70, 6, 2467);
    			attr_dev(div2, "class", "crd-itm-psn");
    			add_location(div2, file$7, 66, 4, 2334);
    			attr_dev(p1, "class", "wgt-dscr-stl");
    			add_location(p1, file$7, 76, 8, 2703);
    			attr_dev(div3, "class", "wgt-dscr-w");
    			add_location(div3, file$7, 75, 6, 2669);
    			attr_dev(input1, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input1, "type", "text");
    			add_location(input1, file$7, 79, 8, 2786);
    			attr_dev(div4, "class", "wgt-w");
    			add_location(div4, file$7, 78, 6, 2757);
    			attr_dev(div5, "class", "crd-itm-psn");
    			add_location(div5, file$7, 74, 4, 2636);
    			attr_dev(p2, "class", "wgt-dscr-stl");
    			add_location(p2, file$7, 84, 8, 2991);
    			attr_dev(div6, "class", "wgt-dscr-w");
    			add_location(div6, file$7, 83, 6, 2957);
    			attr_dev(input2, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input2, "type", "text");
    			add_location(input2, file$7, 87, 8, 3077);
    			attr_dev(div7, "class", "wgt-w");
    			add_location(div7, file$7, 86, 6, 3048);
    			attr_dev(div8, "class", "crd-itm-psn");
    			add_location(div8, file$7, 82, 4, 2924);
    			attr_dev(p3, "class", "wgt-dscr-stl");
    			add_location(p3, file$7, 92, 8, 3284);
    			attr_dev(div9, "class", "wgt-dscr-w");
    			add_location(div9, file$7, 91, 6, 3250);
    			attr_dev(input3, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input3, "type", "text");
    			add_location(input3, file$7, 95, 8, 3379);
    			attr_dev(div10, "class", "wgt-w");
    			add_location(div10, file$7, 94, 6, 3350);
    			attr_dev(div11, "class", "crd-itm-psn");
    			add_location(div11, file$7, 90, 4, 3217);
    			attr_dev(p4, "class", "wgt-dscr-stl");
    			add_location(p4, file$7, 100, 8, 3584);
    			attr_dev(div12, "class", "wgt-dscr-w");
    			add_location(div12, file$7, 99, 6, 3550);
    			attr_dev(input4, "class", "ipt-rnd text-left focus:border-indigo-500");
    			attr_dev(input4, "type", "text");
    			add_location(input4, file$7, 103, 8, 3669);
    			attr_dev(div13, "class", "wgt-w");
    			add_location(div13, file$7, 102, 6, 3640);
    			attr_dev(div14, "class", "crd-itm-psn");
    			add_location(div14, file$7, 98, 4, 3517);
    			attr_dev(button, "class", "btn-lg");
    			add_location(button, file$7, 106, 4, 3807);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div0);
    			append_dev(div0, p0);
    			append_dev(div2, t1);
    			append_dev(div2, div1);
    			append_dev(div1, input0);
    			set_input_value(input0, /*settingsJson*/ ctx[0].mqttServer);
    			insert_dev(target, t2, anchor);
    			insert_dev(target, div5, anchor);
    			append_dev(div5, div3);
    			append_dev(div3, p1);
    			append_dev(div5, t4);
    			append_dev(div5, div4);
    			append_dev(div4, input1);
    			set_input_value(input1, /*settingsJson*/ ctx[0].mqttPort);
    			insert_dev(target, t5, anchor);
    			insert_dev(target, div8, anchor);
    			append_dev(div8, div6);
    			append_dev(div6, p2);
    			append_dev(div8, t7);
    			append_dev(div8, div7);
    			append_dev(div7, input2);
    			set_input_value(input2, /*settingsJson*/ ctx[0].mqttPrefix);
    			insert_dev(target, t8, anchor);
    			insert_dev(target, div11, anchor);
    			append_dev(div11, div9);
    			append_dev(div9, p3);
    			append_dev(div11, t10);
    			append_dev(div11, div10);
    			append_dev(div10, input3);
    			set_input_value(input3, /*settingsJson*/ ctx[0].mqttUser);
    			insert_dev(target, t11, anchor);
    			insert_dev(target, div14, anchor);
    			append_dev(div14, div12);
    			append_dev(div12, p4);
    			append_dev(div14, t13);
    			append_dev(div14, div13);
    			append_dev(div13, input4);
    			set_input_value(input4, /*settingsJson*/ ctx[0].mqttPass);
    			insert_dev(target, t14, anchor);
    			insert_dev(target, button, anchor);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input0, "input", /*input0_input_handler_1*/ ctx[11]),
    					listen_dev(input1, "input", /*input1_input_handler_1*/ ctx[12]),
    					listen_dev(input2, "input", /*input2_input_handler_1*/ ctx[13]),
    					listen_dev(input3, "input", /*input3_input_handler_1*/ ctx[14]),
    					listen_dev(input4, "input", /*input4_input_handler*/ ctx[15]),
    					listen_dev(button, "click", /*click_handler_2*/ ctx[16], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input0.value !== /*settingsJson*/ ctx[0].mqttServer) {
    				set_input_value(input0, /*settingsJson*/ ctx[0].mqttServer);
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input1.value !== /*settingsJson*/ ctx[0].mqttPort) {
    				set_input_value(input1, /*settingsJson*/ ctx[0].mqttPort);
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input2.value !== /*settingsJson*/ ctx[0].mqttPrefix) {
    				set_input_value(input2, /*settingsJson*/ ctx[0].mqttPrefix);
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input3.value !== /*settingsJson*/ ctx[0].mqttUser) {
    				set_input_value(input3, /*settingsJson*/ ctx[0].mqttUser);
    			}

    			if (dirty & /*settingsJson, Object, ssidJson*/ 3 && input4.value !== /*settingsJson*/ ctx[0].mqttPass) {
    				set_input_value(input4, /*settingsJson*/ ctx[0].mqttPass);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div2);
    			if (detaching) detach_dev(t2);
    			if (detaching) detach_dev(div5);
    			if (detaching) detach_dev(t5);
    			if (detaching) detach_dev(div8);
    			if (detaching) detach_dev(t8);
    			if (detaching) detach_dev(div11);
    			if (detaching) detach_dev(t11);
    			if (detaching) detach_dev(div14);
    			if (detaching) detach_dev(t14);
    			if (detaching) detach_dev(button);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot$5.name,
    		type: "slot",
    		source: "(66:2) <Card title=\\\"Подключение к MQTT брокеру\\\">",
    		ctx
    	});

    	return block;
    }

    function create_fragment$7(ctx) {
    	let div;
    	let card0;
    	let t;
    	let card1;
    	let current;

    	card0 = new Card({
    			props: {
    				title: "Подключение к WiFi роутеру",
    				$$slots: { default: [create_default_slot_1$3] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	card1 = new Card({
    			props: {
    				title: "Подключение к MQTT брокеру",
    				$$slots: { default: [create_default_slot$5] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			div = element("div");
    			create_component(card0.$$.fragment);
    			t = space();
    			create_component(card1.$$.fragment);
    			attr_dev(div, "class", "grd-2col1");
    			add_location(div, file$7, 10, 0, 260);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			mount_component(card0, div, null);
    			append_dev(div, t);
    			mount_component(card1, div, null);
    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			const card0_changes = {};

    			if (dirty & /*$$scope, saveSettings, settingsJson, ssidDropdownClick, ssidJson*/ 2097167) {
    				card0_changes.$$scope = { dirty, ctx };
    			}

    			card0.$set(card0_changes);
    			const card1_changes = {};

    			if (dirty & /*$$scope, saveSettings, settingsJson*/ 2097161) {
    				card1_changes.$$scope = { dirty, ctx };
    			}

    			card1.$set(card1_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(card0.$$.fragment, local);
    			transition_in(card1.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(card0.$$.fragment, local);
    			transition_out(card1.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			destroy_component(card0);
    			destroy_component(card1);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$7.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$7($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Connection', slots, []);
    	let { settingsJson } = $$props;
    	let { ssidJson } = $$props;

    	let { ssidDropdownClick = () => {
    		
    	} } = $$props;

    	let { saveSettings = () => {
    		
    	} } = $$props;

    	const writable_props = ['settingsJson', 'ssidJson', 'ssidDropdownClick', 'saveSettings'];

    	Object_1$1.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Connection> was created with unknown prop '${key}'`);
    	});

    	function input0_input_handler() {
    		settingsJson.name = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	function input1_input_handler() {
    		settingsJson.apssid = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	function input2_input_handler() {
    		settingsJson.appass = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	function select_change_handler() {
    		settingsJson.routerssid = select_value(this);
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	const click_handler = () => ssidDropdownClick();

    	function input3_input_handler() {
    		settingsJson.routerpass = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	const click_handler_1 = () => saveSettings();

    	function input0_input_handler_1() {
    		settingsJson.mqttServer = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	function input1_input_handler_1() {
    		settingsJson.mqttPort = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	function input2_input_handler_1() {
    		settingsJson.mqttPrefix = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	function input3_input_handler_1() {
    		settingsJson.mqttUser = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	function input4_input_handler() {
    		settingsJson.mqttPass = this.value;
    		$$invalidate(0, settingsJson);
    		$$invalidate(1, ssidJson);
    	}

    	const click_handler_2 = () => saveSettings();

    	$$self.$$set = $$props => {
    		if ('settingsJson' in $$props) $$invalidate(0, settingsJson = $$props.settingsJson);
    		if ('ssidJson' in $$props) $$invalidate(1, ssidJson = $$props.ssidJson);
    		if ('ssidDropdownClick' in $$props) $$invalidate(2, ssidDropdownClick = $$props.ssidDropdownClick);
    		if ('saveSettings' in $$props) $$invalidate(3, saveSettings = $$props.saveSettings);
    	};

    	$$self.$capture_state = () => ({
    		Card,
    		Alarm,
    		settingsJson,
    		ssidJson,
    		ssidDropdownClick,
    		saveSettings
    	});

    	$$self.$inject_state = $$props => {
    		if ('settingsJson' in $$props) $$invalidate(0, settingsJson = $$props.settingsJson);
    		if ('ssidJson' in $$props) $$invalidate(1, ssidJson = $$props.ssidJson);
    		if ('ssidDropdownClick' in $$props) $$invalidate(2, ssidDropdownClick = $$props.ssidDropdownClick);
    		if ('saveSettings' in $$props) $$invalidate(3, saveSettings = $$props.saveSettings);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		settingsJson,
    		ssidJson,
    		ssidDropdownClick,
    		saveSettings,
    		input0_input_handler,
    		input1_input_handler,
    		input2_input_handler,
    		select_change_handler,
    		click_handler,
    		input3_input_handler,
    		click_handler_1,
    		input0_input_handler_1,
    		input1_input_handler_1,
    		input2_input_handler_1,
    		input3_input_handler_1,
    		input4_input_handler,
    		click_handler_2
    	];
    }

    class Connection extends SvelteComponentDev {
    	constructor(options) {
    		super(options);

    		init(this, options, instance$7, create_fragment$7, safe_not_equal, {
    			settingsJson: 0,
    			ssidJson: 1,
    			ssidDropdownClick: 2,
    			saveSettings: 3
    		});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Connection",
    			options,
    			id: create_fragment$7.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*settingsJson*/ ctx[0] === undefined && !('settingsJson' in props)) {
    			console.warn("<Connection> was created without expected prop 'settingsJson'");
    		}

    		if (/*ssidJson*/ ctx[1] === undefined && !('ssidJson' in props)) {
    			console.warn("<Connection> was created without expected prop 'ssidJson'");
    		}
    	}

    	get settingsJson() {
    		throw new Error("<Connection>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set settingsJson(value) {
    		throw new Error("<Connection>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get ssidJson() {
    		throw new Error("<Connection>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set ssidJson(value) {
    		throw new Error("<Connection>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get ssidDropdownClick() {
    		throw new Error("<Connection>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set ssidDropdownClick(value) {
    		throw new Error("<Connection>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get saveSettings() {
    		throw new Error("<Connection>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set saveSettings(value) {
    		throw new Error("<Connection>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    var commonjsGlobal = typeof globalThis !== 'undefined' ? globalThis : typeof window !== 'undefined' ? window : typeof global !== 'undefined' ? global : typeof self !== 'undefined' ? self : {};

    function createCommonjsModule(fn) {
      var module = { exports: {} };
    	return fn(module, module.exports), module.exports;
    }

    var frappeCharts_min_umd = createCommonjsModule(function (module, exports) {
    !function(t,e){module.exports=e();}(commonjsGlobal,function(){function t(t,e){return "string"==typeof t?(e||document).querySelector(t):t||null}function e(t){var e=t.getBoundingClientRect();return {top:e.top+(document.documentElement.scrollTop||document.body.scrollTop),left:e.left+(document.documentElement.scrollLeft||document.body.scrollLeft)}}function i(t){return null===t.offsetParent}function n(t){var e=t.getBoundingClientRect();return e.top>=0&&e.left>=0&&e.bottom<=(window.innerHeight||document.documentElement.clientHeight)&&e.right<=(window.innerWidth||document.documentElement.clientWidth)}function a(t){var e=window.getComputedStyle(t),i=parseFloat(e.paddingLeft)+parseFloat(e.paddingRight);return t.clientWidth-i}function s(t,e,i){var n=document.createEvent("HTMLEvents");n.initEvent(e,!0,!0);for(var a in i)n[a]=i[a];return t.dispatchEvent(n)}function r(t){return t.titleHeight+t.margins.top+t.paddings.top}function o(t){return t.margins.left+t.paddings.left}function l(t){return t.margins.top+t.margins.bottom+t.paddings.top+t.paddings.bottom+t.titleHeight+t.legendHeight}function u(t){return t.margins.left+t.margins.right+t.paddings.left+t.paddings.right}function h(t){return parseFloat(t.toFixed(2))}function c(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]&&arguments[3];i||(i=n?t[0]:t[t.length-1]);var a=new Array(Math.abs(e)).fill(i);return t=n?a.concat(t):t.concat(a)}function d(t,e){return (t+"").length*e}function p(t,e){return {x:Math.sin(t*Zt)*e,y:Math.cos(t*Zt)*e}}function f(t){var e=arguments.length>1&&void 0!==arguments[1]&&arguments[1];return !Number.isNaN(t)&&(void 0!==t&&(!!Number.isFinite(t)&&!(e&&t<0)))}function v(t){return Number(Math.round(t+"e4")+"e-4")}function g(t){var e=void 0,i=void 0,n=void 0;if(t instanceof Date)return new Date(t.getTime());if("object"!==(void 0===t?"undefined":Ft(t))||null===t)return t;e=Array.isArray(t)?[]:{};for(n in t)i=t[n],e[n]=g(i);return e}function m(t,e){var i=void 0,n=void 0;return t<=e?(i=e-t,n=t):(i=t-e,n=e),[i,n]}function y(t,e){var i=arguments.length>2&&void 0!==arguments[2]?arguments[2]:e.length-t.length;return i>0?t=c(t,i):e=c(e,i),[t,e]}function b(t,e){if(t)return t.length>e?t.slice(0,e-3)+"...":t}function x(t){var e=void 0;if("number"==typeof t)e=t;else if("string"==typeof t&&(e=Number(t),Number.isNaN(e)))return t;var i=Math.floor(Math.log10(Math.abs(e)));if(i<=2)return e;var n=Math.floor(i/3),a=Math.pow(10,i-3*n)*+(e/Math.pow(10,i)).toFixed(1);return Math.round(100*a)/100+" "+["","K","M","B","T"][n]}function k(t,e){for(var i=[],n=0;n<t.length;n++)i.push([t[n],e[n]]);var a=function(t,e){var i=e[0]-t[0],n=e[1]-t[1];return {length:Math.sqrt(Math.pow(i,2)+Math.pow(n,2)),angle:Math.atan2(n,i)}},s=function(t,e,i,n){var s=a(e||t,i||t),r=s.angle+(n?Math.PI:0),o=.2*s.length;return [t[0]+Math.cos(r)*o,t[1]+Math.sin(r)*o]};return function(t,e){return t.reduce(function(t,i,n,a){return 0===n?i[0]+","+i[1]:t+" "+e(i,n,a)},"")}(i,function(t,e,i){var n=s(i[e-1],i[e-2],t),a=s(t,i[e-1],i[e+1],!0);return "C "+n[0]+","+n[1]+" "+a[0]+","+a[1]+" "+t[0]+","+t[1]})}function w(t){return t>255?255:t<0?0:t}function A(t,e){var i=ie(t),n=!1;"#"==i[0]&&(i=i.slice(1),n=!0);var a=parseInt(i,16),s=w((a>>16)+e),r=w((a>>8&255)+e),o=w((255&a)+e);return (n?"#":"")+(o|r<<8|s<<16).toString(16)}function P(t){var e=/(^\s*)(rgb|hsl)(a?)[(]\s*([\d.]+\s*%?)\s*,\s*([\d.]+\s*%?)\s*,\s*([\d.]+\s*%?)\s*(?:,\s*([\d.]+)\s*)?[)]$/i;return /(^\s*)(#)((?:[A-Fa-f0-9]{3}){1,2})$/i.test(t)||e.test(t)}function T(t,e){return "string"==typeof t?(e||document).querySelector(t):t||null}function L(t,e){var i=document.createElementNS("http://www.w3.org/2000/svg",t);for(var n in e){var a=e[n];if("inside"===n)T(a).appendChild(i);else if("around"===n){var s=T(a);s.parentNode.insertBefore(i,s),i.appendChild(s);}else "styles"===n?"object"===(void 0===a?"undefined":Ft(a))&&Object.keys(a).map(function(t){i.style[t]=a[t];}):("className"===n&&(n="class"),"innerHTML"===n?i.textContent=a:i.setAttribute(n,a));}return i}function O(t,e){return L("linearGradient",{inside:t,id:e,x1:0,x2:0,y1:0,y2:1})}function M(t,e,i,n){return L("stop",{inside:t,style:"stop-color: "+i,offset:e,"stop-opacity":n})}function C(t,e,i,n){return L("svg",{className:e,inside:t,width:i,height:n})}function D(t){return L("defs",{inside:t})}function N(t){var e=arguments.length>1&&void 0!==arguments[1]?arguments[1]:"",i=arguments.length>2&&void 0!==arguments[2]?arguments[2]:void 0,n={className:t,transform:e};return i&&(n.inside=i),L("g",n)}function S(t){return L("path",{className:arguments.length>1&&void 0!==arguments[1]?arguments[1]:"",d:t,styles:{stroke:arguments.length>2&&void 0!==arguments[2]?arguments[2]:"none",fill:arguments.length>3&&void 0!==arguments[3]?arguments[3]:"none","stroke-width":arguments.length>4&&void 0!==arguments[4]?arguments[4]:2}})}function E(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:1,s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:0,r=i.x+t.x,o=i.y+t.y,l=i.x+e.x,u=i.y+e.y;return "M"+i.x+" "+i.y+"\n\t\tL"+r+" "+o+"\n\t\tA "+n+" "+n+" 0 "+s+" "+(a?1:0)+"\n\t\t"+l+" "+u+" z"}function _(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:1,s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:0,r=i.x+t.x,o=i.y+t.y,l=i.x+e.x,u=2*i.y,h=i.y+e.y;return "M"+i.x+" "+i.y+"\n\t\tL"+r+" "+o+"\n\t\tA "+n+" "+n+" 0 "+s+" "+(a?1:0)+"\n\t\t"+l+" "+u+" z\n\t\tL"+r+" "+u+"\n\t\tA "+n+" "+n+" 0 "+s+" "+(a?1:0)+"\n\t\t"+l+" "+h+" z"}function z(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:1,s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:0,r=i.x+t.x,o=i.y+t.y,l=i.x+e.x,u=i.y+e.y;return "M"+r+" "+o+"\n\t\tA "+n+" "+n+" 0 "+s+" "+(a?1:0)+"\n\t\t"+l+" "+u}function W(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:1,s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:0,r=i.x+t.x,o=i.y+t.y,l=i.x+e.x,u=2*n+o,h=i.y+t.y;return "M"+r+" "+o+"\n\t\tA "+n+" "+n+" 0 "+s+" "+(a?1:0)+"\n\t\t"+l+" "+u+"\n\t\tM"+r+" "+u+"\n\t\tA "+n+" "+n+" 0 "+s+" "+(a?1:0)+"\n\t\t"+l+" "+h}function j(t,e){var i=arguments.length>2&&void 0!==arguments[2]&&arguments[2],n="path-fill-gradient-"+e+"-"+(i?"lighter":"default"),a=O(t,n),s=[1,.6,.2];return i&&(s=[.4,.2,0]),M(a,"0%",e,s[0]),M(a,"50%",e,s[1]),M(a,"100%",e,s[2]),n}function F(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:Jt,s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:"none";return L("rect",{className:"percentage-bar",x:t,y:e,width:i,height:n,fill:s,styles:{stroke:A(s,-25),"stroke-dasharray":"0, "+(n+i)+", "+i+", "+n,"stroke-width":a}})}function H(t,e,i,n,a){var s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:"none",r=arguments.length>6&&void 0!==arguments[6]?arguments[6]:{},o={className:t,x:e,y:i,width:n,height:n,rx:a,fill:s};return Object.keys(r).map(function(t){o[t]=r[t];}),L("rect",o)}function I(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:"none",a=arguments[4];a=arguments.length>5&&void 0!==arguments[5]&&arguments[5]?b(a,se):a;var s={className:"legend-bar",x:0,y:0,width:i,height:"2px",fill:n},r=L("text",{className:"legend-dataset-text",x:0,y:0,dy:2*re+"px","font-size":1.2*re+"px","text-anchor":"start",fill:le,innerHTML:a}),o=L("g",{transform:"translate("+t+", "+e+")"});return o.appendChild(L("rect",s)),o.appendChild(r),o}function R(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:"none",a=arguments[4];a=arguments.length>5&&void 0!==arguments[5]&&arguments[5]?b(a,se):a;var s={className:"legend-dot",cx:0,cy:0,r:i,fill:n},r=L("text",{className:"legend-dataset-text",x:0,y:0,dx:re+"px",dy:re/3+"px","font-size":1.2*re+"px","text-anchor":"start",fill:le,innerHTML:a}),o=L("g",{transform:"translate("+t+", "+e+")"});return o.appendChild(L("circle",s)),o.appendChild(r),o}function Y(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:{},s=a.fontSize||re;return L("text",{className:t,x:e,y:i,dy:(void 0!==a.dy?a.dy:s/2)+"px","font-size":s+"px",fill:a.fill||le,"text-anchor":a.textAnchor||"start",innerHTML:n})}function B(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:{};a.stroke||(a.stroke=oe);var s=L("line",{className:"line-vertical "+a.className,x1:0,x2:0,y1:i,y2:n,styles:{stroke:a.stroke}}),r=L("text",{x:0,y:i>n?i+ae:i-ae-re,dy:re+"px","font-size":re+"px","text-anchor":"middle",innerHTML:e+""}),o=L("g",{transform:"translate("+t+", 0)"});return o.appendChild(s),o.appendChild(r),o}function V(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:{};a.stroke||(a.stroke=oe),a.lineType||(a.lineType=""),a.shortenNumbers&&(e=x(e));var s=L("line",{className:"line-horizontal "+a.className+("dashed"===a.lineType?"dashed":""),x1:i,x2:n,y1:0,y2:0,styles:{stroke:a.stroke}}),r=L("text",{x:i<n?i-ae:i+ae,y:0,dy:re/2-2+"px","font-size":re+"px","text-anchor":i<n?"end":"start",innerHTML:e+""}),o=L("g",{transform:"translate(0, "+t+")","stroke-opacity":1});return 0!==r&&"0"!==r||(o.style.stroke="rgba(27, 31, 35, 0.6)"),o.appendChild(s),o.appendChild(r),o}function U(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:{};f(t)||(t=0),n.pos||(n.pos="left"),n.offset||(n.offset=0),n.mode||(n.mode="span"),n.stroke||(n.stroke=oe),n.className||(n.className="");var a=-1*ne,s="span"===n.mode?i+ne:0;return "tick"===n.mode&&"right"===n.pos&&(a=i+ne,s=i),a+=n.offset,s+=n.offset,V(t,e,a,s,{stroke:n.stroke,className:n.className,lineType:n.lineType,shortenNumbers:n.shortenNumbers})}function G(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:{};f(t)||(t=0),n.pos||(n.pos="bottom"),n.offset||(n.offset=0),n.mode||(n.mode="span"),n.stroke||(n.stroke=oe),n.className||(n.className="");var a=i+ne,s="span"===n.mode?-1*ne:i;return "tick"===n.mode&&"top"===n.pos&&(a=-1*ne,s=0),B(t,e,a,s,{stroke:n.stroke,className:n.className,lineType:n.lineType})}function q(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:{};n.labelPos||(n.labelPos="right");var a=L("text",{className:"chart-label",x:"left"===n.labelPos?ae:i-d(e,5)-ae,y:0,dy:re/-2+"px","font-size":re+"px","text-anchor":"start",innerHTML:e+""}),s=V(t,"",0,i,{stroke:n.stroke||oe,className:n.className||"",lineType:n.lineType});return s.appendChild(a),s}function X(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:{},s=t-e,r=L("rect",{className:"bar mini",styles:{fill:"rgba(228, 234, 239, 0.49)",stroke:oe,"stroke-dasharray":i+", "+s},x:0,y:0,width:i,height:s});a.labelPos||(a.labelPos="right");var o=L("text",{className:"chart-label",x:"left"===a.labelPos?ae:i-d(n+"",4.5)-ae,y:0,dy:re/-2+"px","font-size":re+"px","text-anchor":"start",innerHTML:n+""}),l=L("g",{transform:"translate(0, "+e+")"});return l.appendChild(r),l.appendChild(o),l}function J(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:"",s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:0,r=arguments.length>6&&void 0!==arguments[6]?arguments[6]:0,o=arguments.length>7&&void 0!==arguments[7]?arguments[7]:{},l=m(e,o.zeroLine),u=Vt(l,2),h=u[0],c=u[1];c-=r,0===h&&(h=o.minHeight,c-=o.minHeight),f(t)||(t=0),f(c)||(c=0),f(h,!0)||(h=0),f(i,!0)||(i=0);var d=L("rect",{className:"bar mini",style:"fill: "+n,"data-point-index":s,x:t,y:c,width:i,height:h});if((a+="")||a.length){d.setAttribute("y",0),d.setAttribute("x",0);var p=L("text",{className:"data-point-value",x:i/2,y:0,dy:re/2*-1+"px","font-size":re+"px","text-anchor":"middle",innerHTML:a}),v=L("g",{"data-point-index":s,transform:"translate("+t+", "+c+")"});return v.appendChild(d),v.appendChild(p),v}return d}function K(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:"",s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:0,r=L("circle",{style:"fill: "+n,"data-point-index":s,cx:t,cy:e,r:i});if((a+="")||a.length){r.setAttribute("cy",0),r.setAttribute("cx",0);var o=L("text",{className:"data-point-value",x:0,y:0,dy:re/2*-1-i+"px","font-size":re+"px","text-anchor":"middle",innerHTML:a}),l=L("g",{"data-point-index":s,transform:"translate("+t+", "+e+")"});return l.appendChild(r),l.appendChild(o),l}return r}function $(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:{},a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:{},s=e.map(function(e,i){return t[i]+","+e}).join("L");n.spline&&(s=k(t,e));var r=S("M"+s,"line-graph-path",i);if(n.heatline){var o=j(a.svgDefs,i);r.style.stroke="url(#"+o+")";}var l={path:r};if(n.regionFill){var u=j(a.svgDefs,i,!0),h="M"+t[0]+","+a.zeroLine+"L"+s+"L"+t.slice(-1)[0]+","+a.zeroLine;l.region=S(h,"region-fill","none","url(#"+u+")");}return l}function Q(t,e,i,n){var a="string"==typeof e?e:e.join(", ");return [t,{transform:i.join(", ")},n,ve,"translate",{transform:a}]}function Z(t,e,i){return Q(t,[i,0],[e,0],pe)}function tt(t,e,i){return Q(t,[0,i],[0,e],pe)}function et(t,e,i,n){var a=e-i,s=t.childNodes[0];return [[s,{height:a,"stroke-dasharray":s.getAttribute("width")+", "+a},pe,ve],Q(t,[0,n],[0,i],pe)]}function it(t,e,i,n){var a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:0,s=m(i,(arguments.length>5&&void 0!==arguments[5]?arguments[5]:{}).zeroLine),r=Vt(s,2),o=r[0],l=r[1];return l-=a,"rect"!==t.nodeName?[[t.childNodes[0],{width:n,height:o},ce,ve],Q(t,t.getAttribute("transform").split("(")[1].slice(0,-1),[e,l],pe)]:[[t,{width:n,height:o,x:e,y:l},ce,ve]]}function nt(t,e,i){return "circle"!==t.nodeName?[Q(t,t.getAttribute("transform").split("(")[1].slice(0,-1),[e,i],pe)]:[[t,{cx:e,cy:i},ce,ve]]}function at(t,e,i,n,a){var s=[],r=i.map(function(t,i){return e[i]+","+t}).join("L");a&&(r=k(e,i));var o=[t.path,{d:"M"+r},de,ve];if(s.push(o),t.region){var l=e[0]+","+n+"L",u="L"+e.slice(-1)[0]+", "+n,h=[t.region,{d:"M"+l+r+u},de,ve];s.push(h);}return s}function st(t,e){return [t,{d:e},ce,ve]}function rt(t,e,i){var n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:"linear",a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:void 0,s=arguments.length>5&&void 0!==arguments[5]?arguments[5]:{},r=t.cloneNode(!0),o=t.cloneNode(!0);for(var l in e){var u=void 0;u="transform"===l?document.createElementNS("http://www.w3.org/2000/svg","animateTransform"):document.createElementNS("http://www.w3.org/2000/svg","animate");var h=s[l]||t.getAttribute(l),c=e[l],d={attributeName:l,from:h,to:c,begin:"0s",dur:i/1e3+"s",values:h+";"+c,keySplines:ge[n],keyTimes:"0;1",calcMode:"spline",fill:"freeze"};a&&(d.type=a);for(var p in d)u.setAttribute(p,d[p]);r.appendChild(u),a?o.setAttribute(l,"translate("+c+")"):o.setAttribute(l,c);}return [r,o]}function ot(t,e){t.style.transform=e,t.style.webkitTransform=e,t.style.msTransform=e,t.style.mozTransform=e,t.style.oTransform=e;}function lt(t,e){var i=[],n=[];e.map(function(t){var e=t[0],a=e.parentNode,s=void 0,r=void 0;t[0]=e;var o=rt.apply(void 0,Ut(t)),l=Vt(o,2);s=l[0],r=l[1],i.push(r),n.push([s,a]),a&&a.replaceChild(s,e);});var a=t.cloneNode(!0);return n.map(function(t,n){t[1]&&(t[1].replaceChild(i[n],t[0]),e[n][0]=i[n]);}),a}function ut(t,e,i){if(0!==i.length){var n=lt(e,i);e.parentNode==t&&(t.removeChild(e),t.appendChild(n)),setTimeout(function(){n.parentNode==t&&(t.removeChild(n),t.appendChild(e));},fe);}}function ht(t,e){var i=document.createElement("a");i.style="display: none";var n=new Blob(e,{type:"image/svg+xml; charset=utf-8"}),a=window.URL.createObjectURL(n);i.href=a,i.download=t,document.body.appendChild(i),i.click(),setTimeout(function(){document.body.removeChild(i),window.URL.revokeObjectURL(a);},300);}function ct(e){var i=e.cloneNode(!0);i.classList.add("chart-container"),i.setAttribute("xmlns","http://www.w3.org/2000/svg"),i.setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink");var n=t.create("style",{innerHTML:me});i.insertBefore(n,i.firstChild);var a=t.create("div");return a.appendChild(i),a.innerHTML}function dt(t){var e=new Date(t);return e.setMinutes(e.getMinutes()-e.getTimezoneOffset()),e}function pt(t){var e=t.getDate(),i=t.getMonth()+1;return [t.getFullYear(),(i>9?"":"0")+i,(e>9?"":"0")+e].join("-")}function ft(t){return new Date(t.getTime())}function vt(t,e){var i=xt(t);return Math.ceil(gt(i,e)/xe)}function gt(t,e){var i=we*ke;return (dt(e)-dt(t))/i}function mt(t,e){return t.getMonth()===e.getMonth()&&t.getFullYear()===e.getFullYear()}function yt(t){var e=arguments.length>1&&void 0!==arguments[1]&&arguments[1],i=Ae[t];return e?i.slice(0,3):i}function bt(t,e){return new Date(e,t+1,0)}function xt(t){var e=ft(t),i=e.getDay();return 0!==i&&kt(e,-1*i),e}function kt(t,e){t.setDate(t.getDate()+e);}function wt(t,e,i){var n=Object.keys(Le).filter(function(e){return t.includes(e)}),a=Le[n[0]];return Object.assign(a,{constants:e,getData:i}),new Te(a)}function At(t){if(0===t)return [0,0];if(isNaN(t))return {mantissa:-6755399441055744,exponent:972};var e=t>0?1:-1;if(!isFinite(t))return {mantissa:4503599627370496*e,exponent:972};t=Math.abs(t);var i=Math.floor(Math.log10(t));return [e*(t/Math.pow(10,i)),i]}function Pt(t){var e=arguments.length>1&&void 0!==arguments[1]?arguments[1]:0,i=Math.ceil(t),n=Math.floor(e),a=i-n,s=a,r=1;a>5&&(a%2!=0&&(a=++i-n),s=a/2,r=2),a<=2&&(r=a/(s=4)),0===a&&(s=5,r=1);for(var o=[],l=0;l<=s;l++)o.push(n+r*l);return o}function Tt(t){var e=arguments.length>1&&void 0!==arguments[1]?arguments[1]:0,i=At(t),n=Vt(i,2),a=n[0],s=n[1],r=e?e/Math.pow(10,s):0,o=Pt(a=a.toFixed(6),r);return o=o.map(function(t){return t*Math.pow(10,s)})}function Lt(t){function e(t,e){for(var i=Tt(t),n=i[1]-i[0],a=0,s=1;a<e;s++)a+=n,i.unshift(-1*a);return i}var i=arguments.length>1&&void 0!==arguments[1]&&arguments[1],n=Math.max.apply(Math,Ut(t)),a=Math.min.apply(Math,Ut(t)),s=[];if(n>=0&&a>=0)At(n)[1],s=i?Tt(n,a):Tt(n);else if(n>0&&a<0){var r=Math.abs(a);n>=r?(At(n)[1],s=e(n,r)):(At(r)[1],s=e(r,n).reverse().map(function(t){return -1*t}));}else if(n<=0&&a<=0){var o=Math.abs(a),l=Math.abs(n);At(o)[1],s=(s=i?Tt(o,l):Tt(o)).reverse().map(function(t){return -1*t});}return s}function Ot(t){var e=Mt(t);return t.indexOf(0)>=0?t.indexOf(0):t[0]>0?-1*t[0]/e:-1*t[t.length-1]/e+(t.length-1)}function Mt(t){return t[1]-t[0]}function Ct(t){return t[t.length-1]-t[0]}function Dt(t,e){return h(e.zeroLine-t*e.scaleMultiplier)}function Nt(t,e){var i=arguments.length>2&&void 0!==arguments[2]&&arguments[2],n=e.reduce(function(e,i){return Math.abs(i-t)<Math.abs(e-t)?i:e},[]);return i?e.indexOf(n):n}function St(t,e){for(var i=Math.max.apply(Math,Ut(t)),n=1/(e-1),a=[],s=0;s<e;s++){var r=i*(n*s);a.push(r);}return a}function Et(t,e){return e.filter(function(e){return e<t}).length}function _t(t,e){t.labels=t.labels||[];var i=t.labels.length,n=t.datasets,a=new Array(i).fill(0);return n||(n=[{values:a}]),n.map(function(t){if(t.values){var n=t.values;n=(n=n.map(function(t){return isNaN(t)?0:t})).length>i?n.slice(0,i):c(n,i-n.length,0),t.values=n;}else t.values=a;t.chartType||(t.chartType=e);}),t.yRegions&&t.yRegions.map(function(t){if(t.end<t.start){var e=[t.end,t.start];t.start=e[0],t.end=e[1];}}),t}function zt(t){var e=t.labels.length,i=new Array(e).fill(0),n={labels:t.labels.slice(0,-1),datasets:t.datasets.map(function(t){return {name:"",values:i.slice(0,-1),chartType:t.chartType}})};return t.yMarkers&&(n.yMarkers=[{value:0,label:""}]),t.yRegions&&(n.yRegions=[{start:0,end:0,label:""}]),n}function Wt(t){var e=arguments.length>1&&void 0!==arguments[1]?arguments[1]:[],i=!(arguments.length>2&&void 0!==arguments[2])||arguments[2],n=t/e.length;n<=0&&(n=1);var a=n/Kt,s=void 0;if(i){var r=Math.max.apply(Math,Ut(e.map(function(t){return t.length})));s=Math.ceil(r/a);}return e.map(function(t,e){return (t+="").length>a&&(i?e%s!=0&&(t=""):t=a-3>0?t.slice(0,a-3)+" ...":t.slice(0,a)+".."),t})}function jt(){var t=arguments.length>0&&void 0!==arguments[0]?arguments[0]:"line",e=arguments[1],i=arguments[2];return "axis-mixed"===t?(i.type="line",new De(e,i)):Se[t]?new Se[t](e,i):void console.error("Undefined chart type: "+t)}!function(t,e){void 0===e&&(e={});var i=e.insertAt;if(t&&"undefined"!=typeof document){var n=document.head||document.getElementsByTagName("head")[0],a=document.createElement("style");a.type="text/css","top"===i&&n.firstChild?n.insertBefore(a,n.firstChild):n.appendChild(a),a.styleSheet?a.styleSheet.cssText=t:a.appendChild(document.createTextNode(t));}}('.chart-container{position:relative;font-family:-apple-system,BlinkMacSystemFont,Segoe UI,Roboto,Oxygen,Ubuntu,Cantarell,Fira Sans,Droid Sans,Helvetica Neue,sans-serif}.chart-container .axis,.chart-container .chart-label{fill:#555b51}.chart-container .axis line,.chart-container .chart-label line{stroke:#dadada}.chart-container .dataset-units circle{stroke:#fff;stroke-width:2}.chart-container .dataset-units path{fill:none;stroke-opacity:1;stroke-width:2px}.chart-container .dataset-path{stroke-width:2px}.chart-container .path-group path{fill:none;stroke-opacity:1;stroke-width:2px}.chart-container line.dashed{stroke-dasharray:5,3}.chart-container .axis-line .specific-value{text-anchor:start}.chart-container .axis-line .y-line{text-anchor:end}.chart-container .axis-line .x-line{text-anchor:middle}.chart-container .legend-dataset-text{fill:#6c7680;font-weight:600}.graph-svg-tip{position:absolute;z-index:99999;padding:10px;font-size:12px;color:#959da5;text-align:center;background:rgba(0,0,0,.8);border-radius:3px}.graph-svg-tip ol,.graph-svg-tip ul{padding-left:0;display:-webkit-box;display:-ms-flexbox;display:flex}.graph-svg-tip ul.data-point-list li{min-width:90px;-webkit-box-flex:1;-ms-flex:1;flex:1;font-weight:600}.graph-svg-tip strong{color:#dfe2e5;font-weight:600}.graph-svg-tip .svg-pointer{position:absolute;height:5px;margin:0 0 0 -5px;content:" ";border:5px solid transparent;border-top-color:rgba(0,0,0,.8)}.graph-svg-tip.comparison{padding:0;text-align:left;pointer-events:none}.graph-svg-tip.comparison .title{display:block;padding:10px;margin:0;font-weight:600;line-height:1;pointer-events:none}.graph-svg-tip.comparison ul{margin:0;white-space:nowrap;list-style:none}.graph-svg-tip.comparison li{display:inline-block;padding:5px 10px}');var Ft="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function(t){return typeof t}:function(t){return t&&"function"==typeof Symbol&&t.constructor===Symbol&&t!==Symbol.prototype?"symbol":typeof t},Ht=(function(t,e){if(!(t instanceof e))throw new TypeError("Cannot call a class as a function")}),It=function(){function t(t,e){for(var i=0;i<e.length;i++){var n=e[i];n.enumerable=n.enumerable||!1,n.configurable=!0,"value"in n&&(n.writable=!0),Object.defineProperty(t,n.key,n);}}return function(e,i,n){return i&&t(e.prototype,i),n&&t(e,n),e}}(),Rt=function t(e,i,n){null===e&&(e=Function.prototype);var a=Object.getOwnPropertyDescriptor(e,i);if(void 0===a){var s=Object.getPrototypeOf(e);return null===s?void 0:t(s,i,n)}if("value"in a)return a.value;var r=a.get;if(void 0!==r)return r.call(n)},Yt=function(t,e){if("function"!=typeof e&&null!==e)throw new TypeError("Super expression must either be null or a function, not "+typeof e);t.prototype=Object.create(e&&e.prototype,{constructor:{value:t,enumerable:!1,writable:!0,configurable:!0}}),e&&(Object.setPrototypeOf?Object.setPrototypeOf(t,e):t.__proto__=e);},Bt=function(t,e){if(!t)throw new ReferenceError("this hasn't been initialised - super() hasn't been called");return !e||"object"!=typeof e&&"function"!=typeof e?t:e},Vt=function(){function t(t,e){var i=[],n=!0,a=!1,s=void 0;try{for(var r,o=t[Symbol.iterator]();!(n=(r=o.next()).done)&&(i.push(r.value),!e||i.length!==e);n=!0);}catch(t){a=!0,s=t;}finally{try{!n&&o.return&&o.return();}finally{if(a)throw s}}return i}return function(e,i){if(Array.isArray(e))return e;if(Symbol.iterator in Object(e))return t(e,i);throw new TypeError("Invalid attempt to destructure non-iterable instance")}}(),Ut=function(t){if(Array.isArray(t)){for(var e=0,i=Array(t.length);e<t.length;e++)i[e]=t[e];return i}return Array.from(t)};t.create=function(e,i){var n=document.createElement(e);for(var a in i){var s=i[a];if("inside"===a)t(s).appendChild(n);else if("around"===a){var r=t(s);r.parentNode.insertBefore(n,r),n.appendChild(r);}else "styles"===a?"object"===(void 0===s?"undefined":Ft(s))&&Object.keys(s).map(function(t){n.style[t]=s[t];}):a in n?n[a]=s:n.setAttribute(a,s);}return n};var Gt={margins:{top:10,bottom:10,left:20,right:20},paddings:{top:20,bottom:40,left:30,right:10},baseHeight:240,titleHeight:20,legendHeight:30,titleFontSize:12},qt=700,Jt=2,Kt=7,$t=["light-blue","blue","violet","red","orange","yellow","green","light-green","purple","magenta","light-grey","dark-grey"],Qt={bar:$t,line:$t,pie:$t,percentage:$t,heatmap:["#ebedf0","#c6e48b","#7bc96f","#239a3b","#196127"],donut:$t},Zt=Math.PI/180,te=function(){function e(t){var i=t.parent,n=void 0===i?null:i,a=t.colors,s=void 0===a?[]:a;Ht(this,e),this.parent=n,this.colors=s,this.titleName="",this.titleValue="",this.listValues=[],this.titleValueFirst=0,this.x=0,this.y=0,this.top=0,this.left=0,this.setup();}return It(e,[{key:"setup",value:function(){this.makeTooltip();}},{key:"refresh",value:function(){this.fill(),this.calcPosition();}},{key:"makeTooltip",value:function(){var e=this;this.container=t.create("div",{inside:this.parent,className:"graph-svg-tip comparison",innerHTML:'<span class="title"></span>\n\t\t\t\t<ul class="data-point-list"></ul>\n\t\t\t\t<div class="svg-pointer"></div>'}),this.hideTip(),this.title=this.container.querySelector(".title"),this.dataPointList=this.container.querySelector(".data-point-list"),this.parent.addEventListener("mouseleave",function(){e.hideTip();});}},{key:"fill",value:function(){var e=this,i=void 0;this.index&&this.container.setAttribute("data-point-index",this.index),i=this.titleValueFirst?"<strong>"+this.titleValue+"</strong>"+this.titleName:this.titleName+"<strong>"+this.titleValue+"</strong>",this.title.innerHTML=i,this.dataPointList.innerHTML="",this.listValues.map(function(i,n){var a=e.colors[n]||"black",s=0===i.formatted||i.formatted?i.formatted:i.value,r=t.create("li",{styles:{"border-top":"3px solid "+a},innerHTML:'<strong style="display: block;">'+(0===s||s?s:"")+"</strong>\n\t\t\t\t\t"+(i.title?i.title:"")});e.dataPointList.appendChild(r);});}},{key:"calcPosition",value:function(){var t=this.container.offsetWidth;this.top=this.y-this.container.offsetHeight-5,this.left=this.x-t/2;var e=this.parent.offsetWidth-t,i=this.container.querySelector(".svg-pointer");if(this.left<0)i.style.left="calc(50% - "+-1*this.left+"px)",this.left=0;else if(this.left>e){var n="calc(50% + "+(this.left-e)+"px)";i.style.left=n,this.left=e;}else i.style.left="50%";}},{key:"setValues",value:function(t,e){var i=arguments.length>2&&void 0!==arguments[2]?arguments[2]:{},n=arguments.length>3&&void 0!==arguments[3]?arguments[3]:[],a=arguments.length>4&&void 0!==arguments[4]?arguments[4]:-1;this.titleName=i.name,this.titleValue=i.value,this.listValues=n,this.x=t,this.y=e,this.titleValueFirst=i.valueFirst||0,this.index=a,this.refresh();}},{key:"hideTip",value:function(){this.container.style.top="0px",this.container.style.left="0px",this.container.style.opacity="0";}},{key:"showTip",value:function(){this.container.style.top=this.top+"px",this.container.style.left=this.left+"px",this.container.style.opacity="1";}}]),e}(),ee={"light-blue":"#7cd6fd",blue:"#5e64ff",violet:"#743ee2",red:"#ff5858",orange:"#ffa00a",yellow:"#feef72",green:"#28a745","light-green":"#98d85b",purple:"#b554ff",magenta:"#ffa3ef",black:"#36114C",grey:"#bdd3e6","light-grey":"#f0f4f7","dark-grey":"#b8c2cc"},ie=function(t){return /rgb[a]{0,1}\([\d, ]+\)/gim.test(t)?/\D+(\d*)\D+(\d*)\D+(\d*)/gim.exec(t).map(function(t,e){return 0!==e?Number(t).toString(16):"#"}).reduce(function(t,e){return ""+t+e}):ee[t]||t},ne=6,ae=4,se=15,re=10,oe="#dadada",le="#555b51",ue={bar:function(t){var e=void 0;"rect"!==t.nodeName&&(e=t.getAttribute("transform"),t=t.childNodes[0]);var i=t.cloneNode();return i.style.fill="#000000",i.style.opacity="0.4",e&&i.setAttribute("transform",e),i},dot:function(t){var e=void 0;"circle"!==t.nodeName&&(e=t.getAttribute("transform"),t=t.childNodes[0]);var i=t.cloneNode(),n=t.getAttribute("r"),a=t.getAttribute("fill");return i.setAttribute("r",parseInt(n)+4),i.setAttribute("fill",a),i.style.opacity="0.6",e&&i.setAttribute("transform",e),i},heat_square:function(t){var e=void 0;"circle"!==t.nodeName&&(e=t.getAttribute("transform"),t=t.childNodes[0]);var i=t.cloneNode(),n=t.getAttribute("r"),a=t.getAttribute("fill");return i.setAttribute("r",parseInt(n)+4),i.setAttribute("fill",a),i.style.opacity="0.6",e&&i.setAttribute("transform",e),i}},he={bar:function(t,e){var i=void 0;"rect"!==t.nodeName&&(i=t.getAttribute("transform"),t=t.childNodes[0]);var n=["x","y","width","height"];Object.values(t.attributes).filter(function(t){return n.includes(t.name)&&t.specified}).map(function(t){e.setAttribute(t.name,t.nodeValue);}),i&&e.setAttribute("transform",i);},dot:function(t,e){var i=void 0;"circle"!==t.nodeName&&(i=t.getAttribute("transform"),t=t.childNodes[0]);var n=["cx","cy"];Object.values(t.attributes).filter(function(t){return n.includes(t.name)&&t.specified}).map(function(t){e.setAttribute(t.name,t.nodeValue);}),i&&e.setAttribute("transform",i);},heat_square:function(t,e){var i=void 0;"circle"!==t.nodeName&&(i=t.getAttribute("transform"),t=t.childNodes[0]);var n=["cx","cy"];Object.values(t.attributes).filter(function(t){return n.includes(t.name)&&t.specified}).map(function(t){e.setAttribute(t.name,t.nodeValue);}),i&&e.setAttribute("transform",i);}},ce=350,de=350,pe=ce,fe=250,ve="easein",ge={ease:"0.25 0.1 0.25 1",linear:"0 0 1 1",easein:"0.1 0.8 0.2 1",easeout:"0 0 0.58 1",easeinout:"0.42 0 0.58 1"},me=".chart-container{position:relative;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI','Roboto','Oxygen','Ubuntu','Cantarell','Fira Sans','Droid Sans','Helvetica Neue',sans-serif}.chart-container .axis,.chart-container .chart-label{fill:#555b51}.chart-container .axis line,.chart-container .chart-label line{stroke:#dadada}.chart-container .dataset-units circle{stroke:#fff;stroke-width:2}.chart-container .dataset-units path{fill:none;stroke-opacity:1;stroke-width:2px}.chart-container .dataset-path{stroke-width:2px}.chart-container .path-group path{fill:none;stroke-opacity:1;stroke-width:2px}.chart-container line.dashed{stroke-dasharray:5,3}.chart-container .axis-line .specific-value{text-anchor:start}.chart-container .axis-line .y-line{text-anchor:end}.chart-container .axis-line .x-line{text-anchor:middle}.chart-container .legend-dataset-text{fill:#6c7680;font-weight:600}.graph-svg-tip{position:absolute;z-index:99999;padding:10px;font-size:12px;color:#959da5;text-align:center;background:rgba(0,0,0,.8);border-radius:3px}.graph-svg-tip ul{padding-left:0;display:flex}.graph-svg-tip ol{padding-left:0;display:flex}.graph-svg-tip ul.data-point-list li{min-width:90px;flex:1;font-weight:600}.graph-svg-tip strong{color:#dfe2e5;font-weight:600}.graph-svg-tip .svg-pointer{position:absolute;height:5px;margin:0 0 0 -5px;content:' ';border:5px solid transparent;border-top-color:rgba(0,0,0,.8)}.graph-svg-tip.comparison{padding:0;text-align:left;pointer-events:none}.graph-svg-tip.comparison .title{display:block;padding:10px;margin:0;font-weight:600;line-height:1;pointer-events:none}.graph-svg-tip.comparison ul{margin:0;white-space:nowrap;list-style:none}.graph-svg-tip.comparison li{display:inline-block;padding:5px 10px}",ye=function(){function e(t,i){if(Ht(this,e),i=g(i),this.parent="string"==typeof t?document.querySelector(t):t,!(this.parent instanceof HTMLElement))throw new Error("No `parent` element to render on was provided.");this.rawChartArgs=i,this.title=i.title||"",this.type=i.type||"",this.realData=this.prepareData(i.data),this.data=this.prepareFirstData(this.realData),this.colors=this.validateColors(i.colors,this.type),this.config={showTooltip:1,showLegend:1,isNavigable:i.isNavigable||0,animate:void 0!==i.animate?i.animate:1,truncateLegends:i.truncateLegends||1},this.measures=JSON.parse(JSON.stringify(Gt));var n=this.measures;this.setMeasures(i),this.title.length||(n.titleHeight=0),this.config.showLegend||(n.legendHeight=0),this.argHeight=i.height||n.baseHeight,this.state={},this.options={},this.initTimeout=qt,this.config.isNavigable&&(this.overlays=[]),this.configure(i);}return It(e,[{key:"prepareData",value:function(t){return t}},{key:"prepareFirstData",value:function(t){return t}},{key:"validateColors",value:function(t,e){var i=[];return (t=(t||[]).concat(Qt[e])).forEach(function(t){var e=ie(t);P(e)?i.push(e):console.warn('"'+t+'" is not a valid color.');}),i}},{key:"setMeasures",value:function(){}},{key:"configure",value:function(){var t=this,e=this.argHeight;this.baseHeight=e,this.height=e-l(this.measures),this.boundDrawFn=function(){return t.draw(!0)},ResizeObserver&&(this.resizeObserver=new ResizeObserver(this.boundDrawFn),this.resizeObserver.observe(this.parent)),window.addEventListener("resize",this.boundDrawFn),window.addEventListener("orientationchange",this.boundDrawFn);}},{key:"destroy",value:function(){this.resizeObserver&&this.resizeObserver.disconnect(),window.removeEventListener("resize",this.boundDrawFn),window.removeEventListener("orientationchange",this.boundDrawFn);}},{key:"setup",value:function(){this.makeContainer(),this.updateWidth(),this.makeTooltip(),this.draw(!1,!0);}},{key:"makeContainer",value:function(){this.parent.innerHTML="";var e={inside:this.parent,className:"chart-container"};this.independentWidth&&(e.styles={width:this.independentWidth+"px"}),this.container=t.create("div",e);}},{key:"makeTooltip",value:function(){this.tip=new te({parent:this.container,colors:this.colors}),this.bindTooltip();}},{key:"bindTooltip",value:function(){}},{key:"draw",value:function(){var t=this,e=arguments.length>0&&void 0!==arguments[0]&&arguments[0],n=arguments.length>1&&void 0!==arguments[1]&&arguments[1];e&&i(this.parent)||(this.updateWidth(),this.calc(e),this.makeChartArea(),this.setupComponents(),this.components.forEach(function(e){return e.setup(t.drawArea)}),this.render(this.components,!1),n&&(this.data=this.realData,setTimeout(function(){t.update(t.data);},this.initTimeout)),this.renderLegend(),this.setupNavigation(n));}},{key:"calc",value:function(){}},{key:"updateWidth",value:function(){this.baseWidth=a(this.parent),this.width=this.baseWidth-u(this.measures);}},{key:"makeChartArea",value:function(){this.svg&&this.container.removeChild(this.svg);var t=this.measures;this.svg=C(this.container,"frappe-chart chart",this.baseWidth,this.baseHeight),this.svgDefs=D(this.svg),this.title.length&&(this.titleEL=Y("title",t.margins.left,t.margins.top,this.title,{fontSize:t.titleFontSize,fill:"#666666",dy:t.titleFontSize}));var e=r(t);this.drawArea=N(this.type+"-chart chart-draw-area","translate("+o(t)+", "+e+")"),this.config.showLegend&&(e+=this.height+t.paddings.bottom,this.legendArea=N("chart-legend","translate("+o(t)+", "+e+")")),this.title.length&&this.svg.appendChild(this.titleEL),this.svg.appendChild(this.drawArea),this.config.showLegend&&this.svg.appendChild(this.legendArea),this.updateTipOffset(o(t),r(t));}},{key:"updateTipOffset",value:function(t,e){this.tip.offset={x:t,y:e};}},{key:"setupComponents",value:function(){this.components=new Map;}},{key:"update",value:function(t){t||console.error("No data to update."),this.data=this.prepareData(t),this.calc(),this.render(this.components,this.config.animate),this.renderLegend();}},{key:"render",value:function(){var t=this,e=arguments.length>0&&void 0!==arguments[0]?arguments[0]:this.components,i=!(arguments.length>1&&void 0!==arguments[1])||arguments[1];this.config.isNavigable&&this.overlays.map(function(t){return t.parentNode.removeChild(t)});var n=[];e.forEach(function(t){n=n.concat(t.update(i));}),n.length>0?(ut(this.container,this.svg,n),setTimeout(function(){e.forEach(function(t){return t.make()}),t.updateNav();},400)):(e.forEach(function(t){return t.make()}),this.updateNav());}},{key:"updateNav",value:function(){this.config.isNavigable&&(this.makeOverlay(),this.bindUnits());}},{key:"renderLegend",value:function(){}},{key:"setupNavigation",value:function(){var t=this,e=arguments.length>0&&void 0!==arguments[0]&&arguments[0];this.config.isNavigable&&e&&(this.bindOverlay(),this.keyActions={13:this.onEnterKey.bind(this),37:this.onLeftArrow.bind(this),38:this.onUpArrow.bind(this),39:this.onRightArrow.bind(this),40:this.onDownArrow.bind(this)},document.addEventListener("keydown",function(e){n(t.container)&&(e=e||window.event,t.keyActions[e.keyCode]&&t.keyActions[e.keyCode]());}));}},{key:"makeOverlay",value:function(){}},{key:"updateOverlay",value:function(){}},{key:"bindOverlay",value:function(){}},{key:"bindUnits",value:function(){}},{key:"onLeftArrow",value:function(){}},{key:"onRightArrow",value:function(){}},{key:"onUpArrow",value:function(){}},{key:"onDownArrow",value:function(){}},{key:"onEnterKey",value:function(){}},{key:"addDataPoint",value:function(){}},{key:"removeDataPoint",value:function(){}},{key:"getDataPoint",value:function(){}},{key:"setCurrentDataPoint",value:function(){}},{key:"updateDataset",value:function(){}},{key:"export",value:function(){var t=ct(this.svg);ht(this.title||"Chart",[t]);}}]),e}(),be=function(t){function e(t,i){return Ht(this,e),Bt(this,(e.__proto__||Object.getPrototypeOf(e)).call(this,t,i))}return Yt(e,t),It(e,[{key:"configure",value:function(t){Rt(e.prototype.__proto__||Object.getPrototypeOf(e.prototype),"configure",this).call(this,t),this.config.formatTooltipY=(t.tooltipOptions||{}).formatTooltipY,this.config.maxSlices=t.maxSlices||20,this.config.maxLegendPoints=t.maxLegendPoints||20;}},{key:"calc",value:function(){var t=this,e=this.state,i=this.config.maxSlices;e.sliceTotals=[];var n=this.data.labels.map(function(e,i){var n=0;return t.data.datasets.map(function(t){n+=t.values[i];}),[n,e]}).filter(function(t){return t[0]>=0}),a=n;if(n.length>i){n.sort(function(t,e){return e[0]-t[0]}),a=n.slice(0,i-1);var s=0;n.slice(i-1).map(function(t){s+=t[0];}),a.push([s,"Rest"]),this.colors[i-1]="grey";}e.labels=[],a.map(function(t){e.sliceTotals.push(v(t[0])),e.labels.push(t[1]);}),e.grandTotal=e.sliceTotals.reduce(function(t,e){return t+e},0),this.center={x:this.width/2,y:this.height/2};}},{key:"renderLegend",value:function(){var t=this,e=this.state;this.legendArea.textContent="",this.legendTotals=e.sliceTotals.slice(0,this.config.maxLegendPoints);var i=0,n=0;this.legendTotals.map(function(a,s){var r=150,o=Math.floor((t.width-u(t.measures))/r);t.legendTotals.length<o&&(r=t.width/t.legendTotals.length),i>o&&(i=0,n+=20);var l=r*i+5,h=t.config.truncateLegends?b(e.labels[s],r/10):e.labels[s],c=t.config.formatTooltipY?t.config.formatTooltipY(a):a,d=R(l,n,5,t.colors[s],h+": "+c,!1);t.legendArea.appendChild(d),i++;});}}]),e}(ye),xe=7,ke=1e3,we=86400,Ae=["January","February","March","April","May","June","July","August","September","October","November","December"],Pe=["Sun","Mon","Tue","Wed","Thu","Fri","Sat"],Te=function(){function t(e){var i=e.layerClass,n=void 0===i?"":i,a=e.layerTransform,s=void 0===a?"":a,r=e.constants,o=e.getData,l=e.makeElements,u=e.animateElements;Ht(this,t),this.layerTransform=s,this.constants=r,this.makeElements=l,this.getData=o,this.animateElements=u,this.store=[],this.labels=[],this.layerClass=n,this.layerClass="function"==typeof this.layerClass?this.layerClass():this.layerClass,this.refresh();}return It(t,[{key:"refresh",value:function(t){this.data=t||this.getData();}},{key:"setup",value:function(t){this.layer=N(this.layerClass,this.layerTransform,t);}},{key:"make",value:function(){this.render(this.data),this.oldData=this.data;}},{key:"render",value:function(t){var e=this;this.store=this.makeElements(t),this.layer.textContent="",this.store.forEach(function(t){e.layer.appendChild(t);}),this.labels.forEach(function(t){e.layer.appendChild(t);});}},{key:"update",value:function(){var t=!(arguments.length>0&&void 0!==arguments[0])||arguments[0];this.refresh();var e=[];return t&&(e=this.animateElements(this.data)||[]),e}}]),t}(),Le={donutSlices:{layerClass:"donut-slices",makeElements:function(t){return t.sliceStrings.map(function(e,i){var n=S(e,"donut-path",t.colors[i],"none",t.strokeWidth);return n.style.transition="transform .3s;",n})},animateElements:function(t){return this.store.map(function(e,i){return st(e,t.sliceStrings[i])})}},pieSlices:{layerClass:"pie-slices",makeElements:function(t){return t.sliceStrings.map(function(e,i){var n=S(e,"pie-path","none",t.colors[i]);return n.style.transition="transform .3s;",n})},animateElements:function(t){return this.store.map(function(e,i){return st(e,t.sliceStrings[i])})}},percentageBars:{layerClass:"percentage-bars",makeElements:function(t){var e=this;return t.xPositions.map(function(i,n){return F(i,0,t.widths[n],e.constants.barHeight,e.constants.barDepth,t.colors[n])})},animateElements:function(t){if(t)return []}},yAxis:{layerClass:"y axis",makeElements:function(t){var e=this;return t.positions.map(function(i,n){return U(i,t.labels[n],e.constants.width,{mode:e.constants.mode,pos:e.constants.pos,shortenNumbers:e.constants.shortenNumbers})})},animateElements:function(t){var e=t.positions,i=t.labels,n=this.oldData.positions,a=this.oldData.labels,s=y(n,e),r=Vt(s,2);n=r[0],e=r[1];var o=y(a,i),l=Vt(o,2);return a=l[0],i=l[1],this.render({positions:n,labels:i}),this.store.map(function(t,i){return tt(t,e[i],n[i])})}},xAxis:{layerClass:"x axis",makeElements:function(t){var e=this;return t.positions.map(function(i,n){return G(i,t.calcLabels[n],e.constants.height,{mode:e.constants.mode,pos:e.constants.pos})})},animateElements:function(t){var e=t.positions,i=t.calcLabels,n=this.oldData.positions,a=this.oldData.calcLabels,s=y(n,e),r=Vt(s,2);n=r[0],e=r[1];var o=y(a,i),l=Vt(o,2);return a=l[0],i=l[1],this.render({positions:n,calcLabels:i}),this.store.map(function(t,i){return Z(t,e[i],n[i])})}},yMarkers:{layerClass:"y-markers",makeElements:function(t){var e=this;return t.map(function(t){return q(t.position,t.label,e.constants.width,{labelPos:t.options.labelPos,mode:"span",lineType:"dashed"})})},animateElements:function(t){var e=y(this.oldData,t),i=Vt(e,2);this.oldData=i[0];var n=(t=i[1]).map(function(t){return t.position}),a=t.map(function(t){return t.label}),s=t.map(function(t){return t.options}),r=this.oldData.map(function(t){return t.position});return this.render(r.map(function(t,e){return {position:r[e],label:a[e],options:s[e]}})),this.store.map(function(t,e){return tt(t,n[e],r[e])})}},yRegions:{layerClass:"y-regions",makeElements:function(t){var e=this;return t.map(function(t){return X(t.startPos,t.endPos,e.constants.width,t.label,{labelPos:t.options.labelPos})})},animateElements:function(t){var e=y(this.oldData,t),i=Vt(e,2);this.oldData=i[0];var n=(t=i[1]).map(function(t){return t.endPos}),a=t.map(function(t){return t.label}),s=t.map(function(t){return t.startPos}),r=t.map(function(t){return t.options}),o=this.oldData.map(function(t){return t.endPos}),l=this.oldData.map(function(t){return t.startPos});this.render(o.map(function(t,e){return {startPos:l[e],endPos:o[e],label:a[e],options:r[e]}}));var u=[];return this.store.map(function(t,e){u=u.concat(et(t,s[e],n[e],o[e]));}),u}},heatDomain:{layerClass:function(){return "heat-domain domain-"+this.constants.index},makeElements:function(t){var e=this,i=this.constants,n=i.index,a=i.colWidth,s=i.rowHeight,r=i.squareSize,o=i.radius,l=i.xTranslate,u=0;return this.serializedSubDomains=[],t.cols.map(function(t,i){1===i&&e.labels.push(Y("domain-name",l,-12,yt(n,!0).toUpperCase(),{fontSize:9})),t.map(function(t,i){if(t.fill){var n={"data-date":t.yyyyMmDd,"data-value":t.dataValue,"data-day":i},a=H("day",l,u,r,o,t.fill,n);e.serializedSubDomains.push(a);}u+=s;}),u=0,l+=a;}),this.serializedSubDomains},animateElements:function(t){if(t)return []}},barGraph:{layerClass:function(){return "dataset-units dataset-bars dataset-"+this.constants.index},makeElements:function(t){var e=this.constants;return this.unitType="bar",this.units=t.yPositions.map(function(i,n){return J(t.xPositions[n],i,t.barWidth,e.color,t.labels[n],n,t.offsets[n],{zeroLine:t.zeroLine,barsWidth:t.barsWidth,minHeight:e.minHeight})}),this.units},animateElements:function(t){var e=t.xPositions,i=t.yPositions,n=t.offsets,a=t.labels,s=this.oldData.xPositions,r=this.oldData.yPositions,o=this.oldData.offsets,l=this.oldData.labels,u=y(s,e),h=Vt(u,2);s=h[0],e=h[1];var c=y(r,i),d=Vt(c,2);r=d[0],i=d[1];var p=y(o,n),f=Vt(p,2);o=f[0],n=f[1];var v=y(l,a),g=Vt(v,2);l=g[0],a=g[1],this.render({xPositions:s,yPositions:r,offsets:o,labels:a,zeroLine:this.oldData.zeroLine,barsWidth:this.oldData.barsWidth,barWidth:this.oldData.barWidth});var m=[];return this.store.map(function(a,s){m=m.concat(it(a,e[s],i[s],t.barWidth,n[s],{zeroLine:t.zeroLine}));}),m}},lineGraph:{layerClass:function(){return "dataset-units dataset-line dataset-"+this.constants.index},makeElements:function(t){var e=this.constants;return this.unitType="dot",this.paths={},e.hideLine||(this.paths=$(t.xPositions,t.yPositions,e.color,{heatline:e.heatline,regionFill:e.regionFill,spline:e.spline},{svgDefs:e.svgDefs,zeroLine:t.zeroLine})),this.units=[],e.hideDots||(this.units=t.yPositions.map(function(i,n){return K(t.xPositions[n],i,t.radius,e.color,e.valuesOverPoints?t.values[n]:"",n)})),Object.values(this.paths).concat(this.units)},animateElements:function(t){var e=t.xPositions,i=t.yPositions,n=t.values,a=this.oldData.xPositions,s=this.oldData.yPositions,r=this.oldData.values,o=y(a,e),l=Vt(o,2);a=l[0],e=l[1];var u=y(s,i),h=Vt(u,2);s=h[0],i=h[1];var c=y(r,n),d=Vt(c,2);r=d[0],n=d[1],this.render({xPositions:a,yPositions:s,values:n,zeroLine:this.oldData.zeroLine,radius:this.oldData.radius});var p=[];return Object.keys(this.paths).length&&(p=p.concat(at(this.paths,e,i,t.zeroLine,this.constants.spline))),this.units.length&&this.units.map(function(t,n){p=p.concat(nt(t,e[n],i[n]));}),p}}},Oe=function(t){function i(t,e){Ht(this,i);var n=Bt(this,(i.__proto__||Object.getPrototypeOf(i)).call(this,t,e));return n.type="percentage",n.setup(),n}return Yt(i,t),It(i,[{key:"setMeasures",value:function(t){var e=this.measures;this.barOptions=t.barOptions||{};var i=this.barOptions;i.height=i.height||20,i.depth=i.depth||Jt,e.paddings.right=30,e.legendHeight=60,e.baseHeight=8*(i.height+.5*i.depth);}},{key:"setupComponents",value:function(){var t=this.state,e=[["percentageBars",{barHeight:this.barOptions.height,barDepth:this.barOptions.depth},function(){return {xPositions:t.xPositions,widths:t.widths,colors:this.colors}}.bind(this)]];this.components=new Map(e.map(function(t){var e=wt.apply(void 0,Ut(t));return [t[0],e]}));}},{key:"calc",value:function(){var t=this;Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"calc",this).call(this);var e=this.state;e.xPositions=[],e.widths=[];var n=0;e.sliceTotals.map(function(i){var a=t.width*i/e.grandTotal;e.widths.push(a),e.xPositions.push(n),n+=a;});}},{key:"makeDataByIndex",value:function(){}},{key:"bindTooltip",value:function(){var t=this,i=this.state;this.container.addEventListener("mousemove",function(n){var a=t.components.get("percentageBars").store,s=n.target;if(a.includes(s)){var r=a.indexOf(s),o=e(t.container),l=e(s),u=l.left-o.left+parseInt(s.getAttribute("width"))/2,h=l.top-o.top,c=(t.formattedLabels&&t.formattedLabels.length>0?t.formattedLabels[r]:t.state.labels[r])+": ",d=i.sliceTotals[r]/i.grandTotal;t.tip.setValues(u,h,{name:c,value:(100*d).toFixed(1)+"%"}),t.tip.showTip();}});}}]),i}(be),Me=function(t){function i(t,e){Ht(this,i);var n=Bt(this,(i.__proto__||Object.getPrototypeOf(i)).call(this,t,e));return n.type="pie",n.initTimeout=0,n.init=1,n.setup(),n}return Yt(i,t),It(i,[{key:"configure",value:function(t){Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"configure",this).call(this,t),this.mouseMove=this.mouseMove.bind(this),this.mouseLeave=this.mouseLeave.bind(this),this.hoverRadio=t.hoverRadio||.1,this.config.startAngle=t.startAngle||0,this.clockWise=t.clockWise||!1;}},{key:"calc",value:function(){var t=this;Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"calc",this).call(this);var e=this.state;this.radius=this.height>this.width?this.center.x:this.center.y;var n=this.radius,a=this.clockWise,s=e.slicesProperties||[];e.sliceStrings=[],e.slicesProperties=[];var r=180-this.config.startAngle;e.sliceTotals.map(function(i,o){var l=r,u=i/e.grandTotal*360,h=u>180?1:0,c=a?-u:u,d=r+=c,f=p(l,n),v=p(d,n),g=t.init&&s[o],m=void 0,y=void 0;t.init?(m=g?g.startPosition:f,y=g?g.endPosition:f):(m=f,y=v);var b=360===u?_(m,y,t.center,t.radius,a,h):E(m,y,t.center,t.radius,a,h);e.sliceStrings.push(b),e.slicesProperties.push({startPosition:f,endPosition:v,value:i,total:e.grandTotal,startAngle:l,endAngle:d,angle:c});}),this.init=0;}},{key:"setupComponents",value:function(){var t=this.state,e=[["pieSlices",{},function(){return {sliceStrings:t.sliceStrings,colors:this.colors}}.bind(this)]];this.components=new Map(e.map(function(t){var e=wt.apply(void 0,Ut(t));return [t[0],e]}));}},{key:"calTranslateByAngle",value:function(t){var e=this.radius,i=this.hoverRadio,n=p(t.startAngle+t.angle/2,e);return "translate3d("+n.x*i+"px,"+n.y*i+"px,0)"}},{key:"hoverSlice",value:function(t,i,n,a){if(t){var s=this.colors[i];if(n){ot(t,this.calTranslateByAngle(this.state.slicesProperties[i])),t.style.fill=A(s,50);var r=e(this.svg),o=a.pageX-r.left+10,l=a.pageY-r.top-10,u=(this.formatted_labels&&this.formatted_labels.length>0?this.formatted_labels[i]:this.state.labels[i])+": ",h=(100*this.state.sliceTotals[i]/this.state.grandTotal).toFixed(1);this.tip.setValues(o,l,{name:u,value:h+"%"}),this.tip.showTip();}else ot(t,"translate3d(0,0,0)"),this.tip.hideTip(),t.style.fill=s;}}},{key:"bindTooltip",value:function(){this.container.addEventListener("mousemove",this.mouseMove),this.container.addEventListener("mouseleave",this.mouseLeave);}},{key:"mouseMove",value:function(t){var e=t.target,i=this.components.get("pieSlices").store,n=this.curActiveSliceIndex,a=this.curActiveSlice;if(i.includes(e)){var s=i.indexOf(e);this.hoverSlice(a,n,!1),this.curActiveSlice=e,this.curActiveSliceIndex=s,this.hoverSlice(e,s,!0,t);}else this.mouseLeave();}},{key:"mouseLeave",value:function(){this.hoverSlice(this.curActiveSlice,this.curActiveSliceIndex,!1);}}]),i}(be),Ce=function(t){function e(t,i){Ht(this,e);var n=Bt(this,(e.__proto__||Object.getPrototypeOf(e)).call(this,t,i));n.type="heatmap",n.countLabel=i.countLabel||"";var a=["Sunday","Monday"],s=a.includes(i.startSubDomain)?i.startSubDomain:"Sunday";return n.startSubDomainIndex=a.indexOf(s),n.setup(),n}return Yt(e,t),It(e,[{key:"setMeasures",value:function(t){var e=this.measures;this.discreteDomains=0===t.discreteDomains?0:1,e.paddings.top=36,e.paddings.bottom=0,e.legendHeight=24,e.baseHeight=12*xe+l(e);var i=this.data,n=this.discreteDomains?12:0;this.independentWidth=12*(vt(i.start,i.end)+n)+u(e);}},{key:"updateWidth",value:function(){var t=this.discreteDomains?12:0,e=this.state.noOfWeeks?this.state.noOfWeeks:52;this.baseWidth=12*(e+t)+u(this.measures);}},{key:"prepareData",value:function(){var t=arguments.length>0&&void 0!==arguments[0]?arguments[0]:this.data;if(t.start&&t.end&&t.start>t.end)throw new Error("Start date cannot be greater than end date.");if(t.start||(t.start=new Date,t.start.setFullYear(t.start.getFullYear()-1)),t.end||(t.end=new Date),t.dataPoints=t.dataPoints||{},parseInt(Object.keys(t.dataPoints)[0])>1e5){var e={};Object.keys(t.dataPoints).forEach(function(i){var n=new Date(i*ke);e[pt(n)]=t.dataPoints[i];}),t.dataPoints=e;}return t}},{key:"calc",value:function(){var t=this.state;t.start=ft(this.data.start),t.end=ft(this.data.end),t.firstWeekStart=ft(t.start),t.noOfWeeks=vt(t.start,t.end),t.distribution=St(Object.values(this.data.dataPoints),5),t.domainConfigs=this.getDomains();}},{key:"setupComponents",value:function(){var t=this,e=this.state,i=this.discreteDomains?0:1,n=e.domainConfigs.map(function(n,a){return ["heatDomain",{index:n.index,colWidth:12,rowHeight:12,squareSize:10,radius:t.rawChartArgs.radius||0,xTranslate:12*e.domainConfigs.filter(function(t,e){return e<a}).map(function(t){return t.cols.length-i}).reduce(function(t,e){return t+e},0)},function(){return e.domainConfigs[a]}.bind(t)]});this.components=new Map(n.map(function(t,e){var i=wt.apply(void 0,Ut(t));return [t[0]+"-"+e,i]}));var a=0;Pe.forEach(function(e,i){if([1,3,5].includes(i)){var n=Y("subdomain-name",-6,a,e,{fontSize:10,dy:8,textAnchor:"end"});t.drawArea.appendChild(n);}a+=12;});}},{key:"update",value:function(t){t||console.error("No data to update."),this.data=this.prepareData(t),this.draw(),this.bindTooltip();}},{key:"bindTooltip",value:function(){var t=this;this.container.addEventListener("mousemove",function(e){t.components.forEach(function(i){var n=i.store,a=e.target;if(n.includes(a)){var s=a.getAttribute("data-value"),r=a.getAttribute("data-date").split("-"),o=yt(parseInt(r[1])-1,!0),l=t.container.getBoundingClientRect(),u=a.getBoundingClientRect(),h=parseInt(e.target.getAttribute("width")),c=u.left-l.left+h/2,d=u.top-l.top,p=s+" "+t.countLabel,f=" on "+o+" "+r[0]+", "+r[2];t.tip.setValues(c,d,{name:f,value:p,valueFirst:1},[]),t.tip.showTip();}});});}},{key:"renderLegend",value:function(){var t=this;this.legendArea.textContent="";var e=0,i=this.rawChartArgs.radius||0,n=Y("subdomain-name",e,12,"Less",{fontSize:11,dy:9});e=30,this.legendArea.appendChild(n),this.colors.slice(0,5).map(function(n,a){var s=H("heatmap-legend-unit",e+15*a,12,10,i,n);t.legendArea.appendChild(s);});var a=Y("subdomain-name",e+75+3,12,"More",{fontSize:11,dy:9});this.legendArea.appendChild(a);}},{key:"getDomains",value:function(){for(var t=this.state,e=[t.start.getMonth(),t.start.getFullYear()],i=e[0],n=e[1],a=[t.end.getMonth(),t.end.getFullYear()],s=a[0]-i+1+12*(a[1]-n),r=[],o=ft(t.start),l=0;l<s;l++){var u=t.end;if(!mt(o,t.end)){var h=[o.getMonth(),o.getFullYear()];u=bt(h[0],h[1]);}r.push(this.getDomainConfig(o,u)),kt(u,1),o=u;}return r}},{key:"getDomainConfig",value:function(t){var e=arguments.length>1&&void 0!==arguments[1]?arguments[1]:"",i=[t.getMonth(),t.getFullYear()],n=i[0],a=i[1],s=xt(t),r={index:n,cols:[]};kt(e=ft(e)||bt(n,a),1);for(var o=vt(s,e),l=[],u=void 0,h=0;h<o;h++)u=this.getCol(s,n),l.push(u),kt(s=new Date(u[xe-1].yyyyMmDd),1);return void 0!==u[xe-1].dataValue&&(kt(s,1),l.push(this.getCol(s,n,!0))),r.cols=l,r}},{key:"getCol",value:function(t,e){for(var i=arguments.length>2&&void 0!==arguments[2]&&arguments[2],n=this.state,a=ft(t),s=[],r=0;r<xe;r++,kt(a,1)){var o={},l=a>=n.start&&a<=n.end;i||a.getMonth()!==e||!l?o.yyyyMmDd=pt(a):o=this.getSubDomainConfig(a),s.push(o);}return s}},{key:"getSubDomainConfig",value:function(t){var e=pt(t),i=this.data.dataPoints[e];return {yyyyMmDd:e,dataValue:i||0,fill:this.colors[Et(i,this.state.distribution)]}}}]),e}(ye),De=function(t){function i(t,e){Ht(this,i);var n=Bt(this,(i.__proto__||Object.getPrototypeOf(i)).call(this,t,e));return n.barOptions=e.barOptions||{},n.lineOptions=e.lineOptions||{},n.type=e.type||"line",n.init=1,n.setup(),n}return Yt(i,t),It(i,[{key:"setMeasures",value:function(){this.data.datasets.length<=1&&(this.config.showLegend=0,this.measures.paddings.bottom=30);}},{key:"configure",value:function(t){Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"configure",this).call(this,t),t.axisOptions=t.axisOptions||{},t.tooltipOptions=t.tooltipOptions||{},this.config.xAxisMode=t.axisOptions.xAxisMode||"span",this.config.yAxisMode=t.axisOptions.yAxisMode||"span",this.config.xIsSeries=t.axisOptions.xIsSeries||0,this.config.shortenYAxisNumbers=t.axisOptions.shortenYAxisNumbers||0,this.config.formatTooltipX=t.tooltipOptions.formatTooltipX,this.config.formatTooltipY=t.tooltipOptions.formatTooltipY,this.config.valuesOverPoints=t.valuesOverPoints;}},{key:"prepareData",value:function(){return _t(arguments.length>0&&void 0!==arguments[0]?arguments[0]:this.data,this.type)}},{key:"prepareFirstData",value:function(){return zt(arguments.length>0&&void 0!==arguments[0]?arguments[0]:this.data)}},{key:"calc",value:function(){var t=arguments.length>0&&void 0!==arguments[0]&&arguments[0];this.calcXPositions(),t||this.calcYAxisParameters(this.getAllYValues(),"line"===this.type),this.makeDataByIndex();}},{key:"calcXPositions",value:function(){var t=this.state,e=this.data.labels;t.datasetLength=e.length,t.unitWidth=this.width/t.datasetLength,t.xOffset=t.unitWidth/2,t.xAxis={labels:e,positions:e.map(function(e,i){return h(t.xOffset+i*t.unitWidth)})};}},{key:"calcYAxisParameters",value:function(t){var e=Lt(t,arguments.length>1&&void 0!==arguments[1]?arguments[1]:"false"),i=this.height/Ct(e),n=Mt(e)*i,a=this.height-Ot(e)*n;this.state.yAxis={labels:e,positions:e.map(function(t){return a-t*i}),scaleMultiplier:i,zeroLine:a},this.calcDatasetPoints(),this.calcYExtremes(),this.calcYRegions();}},{key:"calcDatasetPoints",value:function(){var t=this.state,e=function(e){return e.map(function(e){return Dt(e,t.yAxis)})};t.datasets=this.data.datasets.map(function(t,i){var n=t.values,a=t.cumulativeYs||[];return {name:t.name&&t.name.replace(/<|>|&/g,function(t){return "&"==t?"&amp;":"<"==t?"&lt;":"&gt;"}),index:i,chartType:t.chartType,values:n,yPositions:e(n),cumulativeYs:a,cumulativeYPos:e(a)}});}},{key:"calcYExtremes",value:function(){var t=this.state;if(this.barOptions.stacked)return void(t.yExtremes=t.datasets[t.datasets.length-1].cumulativeYPos);t.yExtremes=new Array(t.datasetLength).fill(9999),t.datasets.map(function(e){e.yPositions.map(function(e,i){e<t.yExtremes[i]&&(t.yExtremes[i]=e);});});}},{key:"calcYRegions",value:function(){var t=this.state;this.data.yMarkers&&(this.state.yMarkers=this.data.yMarkers.map(function(e){return e.position=Dt(e.value,t.yAxis),e.options||(e.options={}),e})),this.data.yRegions&&(this.state.yRegions=this.data.yRegions.map(function(e){return e.startPos=Dt(e.start,t.yAxis),e.endPos=Dt(e.end,t.yAxis),e.options||(e.options={}),e}));}},{key:"getAllYValues",value:function(){var t,e=this,i="values";if(this.barOptions.stacked){i="cumulativeYs";var n=new Array(this.state.datasetLength).fill(0);this.data.datasets.map(function(t,a){var s=e.data.datasets[a].values;t[i]=n=n.map(function(t,e){return t+s[e]});});}var a=this.data.datasets.map(function(t){return t[i]});return this.data.yMarkers&&a.push(this.data.yMarkers.map(function(t){return t.value})),this.data.yRegions&&this.data.yRegions.map(function(t){a.push([t.end,t.start]);}),(t=[]).concat.apply(t,Ut(a))}},{key:"setupComponents",value:function(){var t=this,e=[["yAxis",{mode:this.config.yAxisMode,width:this.width,shortenNumbers:this.config.shortenYAxisNumbers},function(){return this.state.yAxis}.bind(this)],["xAxis",{mode:this.config.xAxisMode,height:this.height},function(){var t=this.state;return t.xAxis.calcLabels=Wt(this.width,t.xAxis.labels,this.config.xIsSeries),t.xAxis}.bind(this)],["yRegions",{width:this.width,pos:"right"},function(){return this.state.yRegions}.bind(this)]],i=this.state.datasets.filter(function(t){return "bar"===t.chartType}),n=this.state.datasets.filter(function(t){return "line"===t.chartType}),a=i.map(function(e){var n=e.index;return ["barGraph-"+e.index,{index:n,color:t.colors[n],stacked:t.barOptions.stacked,valuesOverPoints:t.config.valuesOverPoints,minHeight:0*t.height},function(){var t=this.state,e=t.datasets[n],a=this.barOptions.stacked,s=this.barOptions.spaceRatio||.5,r=t.unitWidth*(1-s),o=r/(a?1:i.length),l=t.xAxis.positions.map(function(t){return t-r/2});a||(l=l.map(function(t){return t+o*n}));var u=new Array(t.datasetLength).fill("");this.config.valuesOverPoints&&(u=a&&e.index===t.datasets.length-1?e.cumulativeYs:e.values);var h=new Array(t.datasetLength).fill(0);return a&&(h=e.yPositions.map(function(t,i){return t-e.cumulativeYPos[i]})),{xPositions:l,yPositions:e.yPositions,offsets:h,labels:u,zeroLine:t.yAxis.zeroLine,barsWidth:r,barWidth:o}}.bind(t)]}),s=n.map(function(e){var i=e.index;return ["lineGraph-"+e.index,{index:i,color:t.colors[i],svgDefs:t.svgDefs,heatline:t.lineOptions.heatline,regionFill:t.lineOptions.regionFill,spline:t.lineOptions.spline,hideDots:t.lineOptions.hideDots,hideLine:t.lineOptions.hideLine,valuesOverPoints:t.config.valuesOverPoints},function(){var t=this.state,e=t.datasets[i],n=t.yAxis.positions[0]<t.yAxis.zeroLine?t.yAxis.positions[0]:t.yAxis.zeroLine;return {xPositions:t.xAxis.positions,yPositions:e.yPositions,values:e.values,zeroLine:n,radius:this.lineOptions.dotSize||4}}.bind(t)]}),r=[["yMarkers",{width:this.width,pos:"right"},function(){return this.state.yMarkers}.bind(this)]];e=e.concat(a,s,r);var o=["yMarkers","yRegions"];this.dataUnitComponents=[],this.components=new Map(e.filter(function(e){return !o.includes(e[0])||t.state[e[0]]}).map(function(e){var i=wt.apply(void 0,Ut(e));return (e[0].includes("lineGraph")||e[0].includes("barGraph"))&&t.dataUnitComponents.push(i),[e[0],i]}));}},{key:"makeDataByIndex",value:function(){var t=this;this.dataByIndex={};var e=this.state,i=this.config.formatTooltipX,n=this.config.formatTooltipY;e.xAxis.labels.map(function(a,s){var r=t.state.datasets.map(function(e,i){var a=e.values[s];return {title:e.name,value:a,yPos:e.yPositions[s],color:t.colors[i],formatted:n?n(a):a}});t.dataByIndex[s]={label:a,formattedLabel:i?i(a):a,xPos:e.xAxis.positions[s],values:r,yExtreme:e.yExtremes[s]};});}},{key:"bindTooltip",value:function(){var t=this;this.container.addEventListener("mousemove",function(i){var n=t.measures,a=e(t.container),s=i.pageX-a.left-o(n),l=i.pageY-a.top;l<t.height+r(n)&&l>r(n)?t.mapTooltipXPosition(s):t.tip.hideTip();});}},{key:"mapTooltipXPosition",value:function(t){var e=this.state;if(e.yExtremes){var i=Nt(t,e.xAxis.positions,!0);if(i>=0){var n=this.dataByIndex[i];this.tip.setValues(n.xPos+this.tip.offset.x,n.yExtreme+this.tip.offset.y,{name:n.formattedLabel,value:""},n.values,i),this.tip.showTip();}}}},{key:"renderLegend",value:function(){var t=this,e=this.data;e.datasets.length>1&&(this.legendArea.textContent="",e.datasets.map(function(e,i){var n=I(100*i,"0",100,t.colors[i],e.name,t.config.truncateLegends);t.legendArea.appendChild(n);}));}},{key:"makeOverlay",value:function(){var t=this;if(this.init)return void(this.init=0);this.overlayGuides&&this.overlayGuides.forEach(function(t){var e=t.overlay;e.parentNode.removeChild(e);}),this.overlayGuides=this.dataUnitComponents.map(function(t){return {type:t.unitType,overlay:void 0,units:t.units}}),void 0===this.state.currentIndex&&(this.state.currentIndex=this.state.datasetLength-1),this.overlayGuides.map(function(e){var i=e.units[t.state.currentIndex];e.overlay=ue[e.type](i),t.drawArea.appendChild(e.overlay);});}},{key:"updateOverlayGuides",value:function(){this.overlayGuides&&this.overlayGuides.forEach(function(t){var e=t.overlay;e.parentNode.removeChild(e);});}},{key:"bindOverlay",value:function(){var t=this;this.parent.addEventListener("data-select",function(){t.updateOverlay();});}},{key:"bindUnits",value:function(){var t=this;this.dataUnitComponents.map(function(e){e.units.map(function(e){e.addEventListener("click",function(){var i=e.getAttribute("data-point-index");t.setCurrentDataPoint(i);});});}),this.tip.container.addEventListener("click",function(){var e=t.tip.container.getAttribute("data-point-index");t.setCurrentDataPoint(e);});}},{key:"updateOverlay",value:function(){var t=this;this.overlayGuides.map(function(e){var i=e.units[t.state.currentIndex];he[e.type](i,e.overlay);});}},{key:"onLeftArrow",value:function(){this.setCurrentDataPoint(this.state.currentIndex-1);}},{key:"onRightArrow",value:function(){this.setCurrentDataPoint(this.state.currentIndex+1);}},{key:"getDataPoint",value:function(){var t=arguments.length>0&&void 0!==arguments[0]?arguments[0]:this.state.currentIndex,e=this.state;return {index:t,label:e.xAxis.labels[t],values:e.datasets.map(function(e){return e.values[t]})}}},{key:"setCurrentDataPoint",value:function(t){var e=this.state;(t=parseInt(t))<0&&(t=0),t>=e.xAxis.labels.length&&(t=e.xAxis.labels.length-1),t!==e.currentIndex&&(e.currentIndex=t,s(this.parent,"data-select",this.getDataPoint()));}},{key:"addDataPoint",value:function(t,e){var n=arguments.length>2&&void 0!==arguments[2]?arguments[2]:this.state.datasetLength;Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"addDataPoint",this).call(this,t,e,n),this.data.labels.splice(n,0,t),this.data.datasets.map(function(t,i){t.values.splice(n,0,e[i]);}),this.update(this.data);}},{key:"removeDataPoint",value:function(){var t=arguments.length>0&&void 0!==arguments[0]?arguments[0]:this.state.datasetLength-1;this.data.labels.length<=1||(Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"removeDataPoint",this).call(this,t),this.data.labels.splice(t,1),this.data.datasets.map(function(e){e.values.splice(t,1);}),this.update(this.data));}},{key:"updateDataset",value:function(t){var e=arguments.length>1&&void 0!==arguments[1]?arguments[1]:0;this.data.datasets[e].values=t,this.update(this.data);}},{key:"updateDatasets",value:function(t){this.data.datasets.map(function(e,i){t[i]&&(e.values=t[i]);}),this.update(this.data);}}]),i}(ye),Ne=function(t){function i(t,e){Ht(this,i);var n=Bt(this,(i.__proto__||Object.getPrototypeOf(i)).call(this,t,e));return n.type="donut",n.initTimeout=0,n.init=1,n.setup(),n}return Yt(i,t),It(i,[{key:"configure",value:function(t){Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"configure",this).call(this,t),this.mouseMove=this.mouseMove.bind(this),this.mouseLeave=this.mouseLeave.bind(this),this.hoverRadio=t.hoverRadio||.1,this.config.startAngle=t.startAngle||0,this.clockWise=t.clockWise||!1,this.strokeWidth=t.strokeWidth||30;}},{key:"calc",value:function(){var t=this;Rt(i.prototype.__proto__||Object.getPrototypeOf(i.prototype),"calc",this).call(this);var e=this.state;this.radius=this.height>this.width?this.center.x-this.strokeWidth/2:this.center.y-this.strokeWidth/2;var n=this.radius,a=this.clockWise,s=e.slicesProperties||[];e.sliceStrings=[],e.slicesProperties=[];var r=180-this.config.startAngle;e.sliceTotals.map(function(i,o){var l=r,u=i/e.grandTotal*360,h=u>180?1:0,c=a?-u:u,d=r+=c,f=p(l,n),v=p(d,n),g=t.init&&s[o],m=void 0,y=void 0;t.init?(m=g?g.startPosition:f,y=g?g.endPosition:f):(m=f,y=v);var b=360===u?W(m,y,t.center,t.radius,t.clockWise,h):z(m,y,t.center,t.radius,t.clockWise,h);e.sliceStrings.push(b),e.slicesProperties.push({startPosition:f,endPosition:v,value:i,total:e.grandTotal,startAngle:l,endAngle:d,angle:c});}),this.init=0;}},{key:"setupComponents",value:function(){var t=this.state,e=[["donutSlices",{},function(){return {sliceStrings:t.sliceStrings,colors:this.colors,strokeWidth:this.strokeWidth}}.bind(this)]];this.components=new Map(e.map(function(t){var e=wt.apply(void 0,Ut(t));return [t[0],e]}));}},{key:"calTranslateByAngle",value:function(t){var e=this.radius,i=this.hoverRadio,n=p(t.startAngle+t.angle/2,e);return "translate3d("+n.x*i+"px,"+n.y*i+"px,0)"}},{key:"hoverSlice",value:function(t,i,n,a){if(t){var s=this.colors[i];if(n){ot(t,this.calTranslateByAngle(this.state.slicesProperties[i])),t.style.stroke=A(s,50);var r=e(this.svg),o=a.pageX-r.left+10,l=a.pageY-r.top-10,u=(this.formatted_labels&&this.formatted_labels.length>0?this.formatted_labels[i]:this.state.labels[i])+": ",h=(100*this.state.sliceTotals[i]/this.state.grandTotal).toFixed(1);this.tip.setValues(o,l,{name:u,value:h+"%"}),this.tip.showTip();}else ot(t,"translate3d(0,0,0)"),this.tip.hideTip(),t.style.stroke=s;}}},{key:"bindTooltip",value:function(){this.container.addEventListener("mousemove",this.mouseMove),this.container.addEventListener("mouseleave",this.mouseLeave);}},{key:"mouseMove",value:function(t){var e=t.target,i=this.components.get("donutSlices").store,n=this.curActiveSliceIndex,a=this.curActiveSlice;if(i.includes(e)){var s=i.indexOf(e);this.hoverSlice(a,n,!1),this.curActiveSlice=e,this.curActiveSliceIndex=s,this.hoverSlice(e,s,!0,t);}else this.mouseLeave();}},{key:"mouseLeave",value:function(){this.hoverSlice(this.curActiveSlice,this.curActiveSliceIndex,!1);}}]),i}(be),Se={bar:De,line:De,percentage:Oe,heatmap:Ce,pie:Me,donut:Ne},Ee=function t(e,i){return Ht(this,t),jt(i.type,e,i)},_e=Object.freeze({Chart:Ee,PercentageChart:Oe,PieChart:Me,Heatmap:Ce,AxisChart:De}),ze={};return ze.NAME="Frappe Charts",ze.VERSION="1.6.2",ze=Object.assign({},ze,_e)});

    });

    /* node_modules\svelte-frappe-charts\src\components\base.svelte generated by Svelte v3.46.4 */
    const file$6 = "node_modules\\svelte-frappe-charts\\src\\components\\base.svelte";

    function create_fragment$6(ctx) {
    	let div;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			div = element("div");
    			add_location(div, file$6, 89, 0, 2072);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			/*div_binding*/ ctx[18](div);

    			if (!mounted) {
    				dispose = listen_dev(div, "data-select", /*data_select_handler*/ ctx[17], false, false, false);
    				mounted = true;
    			}
    		},
    		p: noop,
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			/*div_binding*/ ctx[18](null);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$6.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$6($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Base', slots, []);

    	let { data = {
    		labels: [],
    		datasets: [{ values: [] }],
    		yMarkers: {},
    		yRegions: []
    	} } = $$props;

    	let { title = '' } = $$props;
    	let { type = 'line' } = $$props;
    	let { height = 300 } = $$props;
    	let { animate = true } = $$props;
    	let { axisOptions = {} } = $$props;
    	let { barOptions = {} } = $$props;
    	let { lineOptions = {} } = $$props;
    	let { tooltipOptions = {} } = $$props;
    	let { colors = [] } = $$props;
    	let { valuesOverPoints = 0 } = $$props;
    	let { isNavigable = false } = $$props;
    	let { maxSlices = 3 } = $$props;

    	/**
     *  COMPONENT
     */
    	//  The Chart returned from frappe
    	let chart = null;

    	//  DOM node for frappe to latch onto
    	let chartRef;

    	//  Helper HOF for calling a fn only if chart exists
    	function ifChartThen(fn) {
    		return function ifChart(...args) {
    			if (chart) {
    				return fn(...args);
    			}
    		};
    	}

    	const addDataPoint = ifChartThen((label, valueFromEachDataset, index) => chart.addDataPoint(label, valueFromEachDataset, index));
    	const removeDataPoint = ifChartThen(index => chart.removeDataPoint(index));
    	const exportChart = ifChartThen(() => chart.export());

    	//  Update the chart when incoming data changes
    	const updateChart = ifChartThen(newData => chart.update(newData));

    	/**
     *  Handle initializing the chart when this Svelte component mounts
     */
    	onMount(() => {
    		chart = new frappeCharts_min_umd.Chart(chartRef,
    		{
    				data,
    				title,
    				type,
    				height,
    				animate,
    				colors,
    				axisOptions,
    				barOptions,
    				lineOptions,
    				tooltipOptions,
    				valuesOverPoints,
    				isNavigable,
    				maxSlices
    			});
    	});

    	//  Mark Chart references for garbage collection when component is unmounted
    	onDestroy(() => {
    		chart = null;
    	});

    	const writable_props = [
    		'data',
    		'title',
    		'type',
    		'height',
    		'animate',
    		'axisOptions',
    		'barOptions',
    		'lineOptions',
    		'tooltipOptions',
    		'colors',
    		'valuesOverPoints',
    		'isNavigable',
    		'maxSlices'
    	];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Base> was created with unknown prop '${key}'`);
    	});

    	function data_select_handler(event) {
    		bubble.call(this, $$self, event);
    	}

    	function div_binding($$value) {
    		binding_callbacks[$$value ? 'unshift' : 'push'](() => {
    			chartRef = $$value;
    			$$invalidate(0, chartRef);
    		});
    	}

    	$$self.$$set = $$props => {
    		if ('data' in $$props) $$invalidate(1, data = $$props.data);
    		if ('title' in $$props) $$invalidate(2, title = $$props.title);
    		if ('type' in $$props) $$invalidate(3, type = $$props.type);
    		if ('height' in $$props) $$invalidate(4, height = $$props.height);
    		if ('animate' in $$props) $$invalidate(5, animate = $$props.animate);
    		if ('axisOptions' in $$props) $$invalidate(6, axisOptions = $$props.axisOptions);
    		if ('barOptions' in $$props) $$invalidate(7, barOptions = $$props.barOptions);
    		if ('lineOptions' in $$props) $$invalidate(8, lineOptions = $$props.lineOptions);
    		if ('tooltipOptions' in $$props) $$invalidate(9, tooltipOptions = $$props.tooltipOptions);
    		if ('colors' in $$props) $$invalidate(10, colors = $$props.colors);
    		if ('valuesOverPoints' in $$props) $$invalidate(11, valuesOverPoints = $$props.valuesOverPoints);
    		if ('isNavigable' in $$props) $$invalidate(12, isNavigable = $$props.isNavigable);
    		if ('maxSlices' in $$props) $$invalidate(13, maxSlices = $$props.maxSlices);
    	};

    	$$self.$capture_state = () => ({
    		onMount,
    		onDestroy,
    		Chart: frappeCharts_min_umd.Chart,
    		data,
    		title,
    		type,
    		height,
    		animate,
    		axisOptions,
    		barOptions,
    		lineOptions,
    		tooltipOptions,
    		colors,
    		valuesOverPoints,
    		isNavigable,
    		maxSlices,
    		chart,
    		chartRef,
    		ifChartThen,
    		addDataPoint,
    		removeDataPoint,
    		exportChart,
    		updateChart
    	});

    	$$self.$inject_state = $$props => {
    		if ('data' in $$props) $$invalidate(1, data = $$props.data);
    		if ('title' in $$props) $$invalidate(2, title = $$props.title);
    		if ('type' in $$props) $$invalidate(3, type = $$props.type);
    		if ('height' in $$props) $$invalidate(4, height = $$props.height);
    		if ('animate' in $$props) $$invalidate(5, animate = $$props.animate);
    		if ('axisOptions' in $$props) $$invalidate(6, axisOptions = $$props.axisOptions);
    		if ('barOptions' in $$props) $$invalidate(7, barOptions = $$props.barOptions);
    		if ('lineOptions' in $$props) $$invalidate(8, lineOptions = $$props.lineOptions);
    		if ('tooltipOptions' in $$props) $$invalidate(9, tooltipOptions = $$props.tooltipOptions);
    		if ('colors' in $$props) $$invalidate(10, colors = $$props.colors);
    		if ('valuesOverPoints' in $$props) $$invalidate(11, valuesOverPoints = $$props.valuesOverPoints);
    		if ('isNavigable' in $$props) $$invalidate(12, isNavigable = $$props.isNavigable);
    		if ('maxSlices' in $$props) $$invalidate(13, maxSlices = $$props.maxSlices);
    		if ('chart' in $$props) chart = $$props.chart;
    		if ('chartRef' in $$props) $$invalidate(0, chartRef = $$props.chartRef);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	$$self.$$.update = () => {
    		if ($$self.$$.dirty & /*data*/ 2) {
    			updateChart(data);
    		}
    	};

    	return [
    		chartRef,
    		data,
    		title,
    		type,
    		height,
    		animate,
    		axisOptions,
    		barOptions,
    		lineOptions,
    		tooltipOptions,
    		colors,
    		valuesOverPoints,
    		isNavigable,
    		maxSlices,
    		addDataPoint,
    		removeDataPoint,
    		exportChart,
    		data_select_handler,
    		div_binding
    	];
    }

    class Base extends SvelteComponentDev {
    	constructor(options) {
    		super(options);

    		init(this, options, instance$6, create_fragment$6, safe_not_equal, {
    			data: 1,
    			title: 2,
    			type: 3,
    			height: 4,
    			animate: 5,
    			axisOptions: 6,
    			barOptions: 7,
    			lineOptions: 8,
    			tooltipOptions: 9,
    			colors: 10,
    			valuesOverPoints: 11,
    			isNavigable: 12,
    			maxSlices: 13,
    			addDataPoint: 14,
    			removeDataPoint: 15,
    			exportChart: 16
    		});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Base",
    			options,
    			id: create_fragment$6.name
    		});
    	}

    	get data() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set data(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get title() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set title(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get type() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set type(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get height() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set height(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get animate() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set animate(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get axisOptions() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set axisOptions(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get barOptions() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set barOptions(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get lineOptions() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set lineOptions(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get tooltipOptions() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set tooltipOptions(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get colors() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set colors(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get valuesOverPoints() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set valuesOverPoints(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get isNavigable() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set isNavigable(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get maxSlices() {
    		throw new Error("<Base>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set maxSlices(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get addDataPoint() {
    		return this.$$.ctx[14];
    	}

    	set addDataPoint(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get removeDataPoint() {
    		return this.$$.ctx[15];
    	}

    	set removeDataPoint(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get exportChart() {
    		return this.$$.ctx[16];
    	}

    	set exportChart(value) {
    		throw new Error("<Base>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    var Base$1 = Base;

    /* src\pages\Utilities.svelte generated by Svelte v3.46.4 */

    const { console: console_1$2 } = globals;
    const file$5 = "src\\pages\\Utilities.svelte";

    // (47:0) <Alarm>
    function create_default_slot_1$2(ctx) {
    	let button;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			button = element("button");
    			button.textContent = `${"Проверить"}`;
    			attr_dev(button, "class", "btn-lg");
    			add_location(button, file$5, 47, 2, 1370);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, button, anchor);

    			if (!mounted) {
    				dispose = listen_dev(button, "click", /*click_handler*/ ctx[3], false, false, false);
    				mounted = true;
    			}
    		},
    		p: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(button);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_1$2.name,
    		type: "slot",
    		source: "(47:0) <Alarm>",
    		ctx
    	});

    	return block;
    }

    // (51:0) <Alarm>
    function create_default_slot$4(ctx) {
    	let chart;
    	let current;

    	chart = new Base$1({
    			props: { data: /*datachart*/ ctx[0], type: "line" },
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(chart.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(chart, target, anchor);
    			current = true;
    		},
    		p: noop,
    		i: function intro(local) {
    			if (current) return;
    			transition_in(chart.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(chart.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(chart, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot$4.name,
    		type: "slot",
    		source: "(51:0) <Alarm>",
    		ctx
    	});

    	return block;
    }

    function create_fragment$5(ctx) {
    	let alarm0;
    	let t;
    	let alarm1;
    	let current;

    	alarm0 = new Alarm({
    			props: {
    				$$slots: { default: [create_default_slot_1$2] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	alarm1 = new Alarm({
    			props: {
    				$$slots: { default: [create_default_slot$4] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(alarm0.$$.fragment);
    			t = space();
    			create_component(alarm1.$$.fragment);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			mount_component(alarm0, target, anchor);
    			insert_dev(target, t, anchor);
    			mount_component(alarm1, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			const alarm0_changes = {};

    			if (dirty & /*$$scope*/ 64) {
    				alarm0_changes.$$scope = { dirty, ctx };
    			}

    			alarm0.$set(alarm0_changes);
    			const alarm1_changes = {};

    			if (dirty & /*$$scope*/ 64) {
    				alarm1_changes.$$scope = { dirty, ctx };
    			}

    			alarm1.$set(alarm1_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(alarm0.$$.fragment, local);
    			transition_in(alarm1.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(alarm0.$$.fragment, local);
    			transition_out(alarm1.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(alarm0, detaching);
    			if (detaching) detach_dev(t);
    			destroy_component(alarm1, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$5.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function joinWithoutDupes(A, B) {
    	let output = [];
    	const a = new Set(A.map(x => x.item));
    	const b = new Set(B.map(x => x.item));
    	output = [...A.filter(x => !b.has(x.item)), ...B.filter(x => !a.has(x.item))];
    	console.log(output);
    }

    function joinWithoutDupesAndRmooving(A, B) {
    	var ids = new Set(A.map(d => d.ID));
    	let output = [...A, ...B.filter(d => !ids.has(d.ID))];
    	console.log(output);
    }

    function instance$5($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Utilities', slots, []);

    	let datachart = {
    		labels: ["Sun", "Mon", "Tues", "Wed", "Thurs", "Fri", "Sat"],
    		datasets: [{ values: [10, 12, 3, 9, 8, 15, 9] }]
    	};

    	//объединение двух массивов с удалением дубликатов полностью
    	let a = [{ item: "1", description: "lorem" }, { item: "2", description: "impsum" }];

    	let b = [{ item: "2", description: "dolor" }, { item: "4", description: "enum" }];

    	//объединение двух массивов с удалением дубликатов, оставляя один из дубликатов
    	let c = [{ ID: "1", description: "lorem" }, { ID: "2", description: "impsum" }];

    	let d = [{ ID: "2", description: "dolor" }, { ID: "4", description: "enum" }];
    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console_1$2.warn(`<Utilities> was created with unknown prop '${key}'`);
    	});

    	const click_handler = () => joinWithoutDupesAndRmooving(c, d);

    	$$self.$capture_state = () => ({
    		Card,
    		Alarm,
    		Chart: Base$1,
    		datachart,
    		a,
    		b,
    		joinWithoutDupes,
    		c,
    		d,
    		joinWithoutDupesAndRmooving
    	});

    	$$self.$inject_state = $$props => {
    		if ('datachart' in $$props) $$invalidate(0, datachart = $$props.datachart);
    		if ('a' in $$props) a = $$props.a;
    		if ('b' in $$props) b = $$props.b;
    		if ('c' in $$props) $$invalidate(1, c = $$props.c);
    		if ('d' in $$props) $$invalidate(2, d = $$props.d);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [datachart, c, d, click_handler];
    }

    class Utilities extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$5, create_fragment$5, safe_not_equal, {});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Utilities",
    			options,
    			id: create_fragment$5.name
    		});
    	}
    }

    /* src\pages\Log.svelte generated by Svelte v3.46.4 */
    const file$4 = "src\\pages\\Log.svelte";

    function get_each_context$2(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[1] = list[i];
    	child_ctx[3] = i;
    	return child_ctx;
    }

    // (8:2) {#each coreMessages as message, i}
    function create_each_block$2(ctx) {
    	let div;
    	let t_value = /*message*/ ctx[1].msg + "";
    	let t;
    	let div_class_value;

    	const block = {
    		c: function create() {
    			div = element("div");
    			t = text(t_value);

    			attr_dev(div, "class", div_class_value = /*message*/ ctx[1].msg.toString().includes("[E]")
    			? "text-red-500"
    			: "text-black");

    			add_location(div, file$4, 8, 4, 166);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			append_dev(div, t);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*coreMessages*/ 1 && t_value !== (t_value = /*message*/ ctx[1].msg + "")) set_data_dev(t, t_value);

    			if (dirty & /*coreMessages*/ 1 && div_class_value !== (div_class_value = /*message*/ ctx[1].msg.toString().includes("[E]")
    			? "text-red-500"
    			: "text-black")) {
    				attr_dev(div, "class", div_class_value);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block$2.name,
    		type: "each",
    		source: "(8:2) {#each coreMessages as message, i}",
    		ctx
    	});

    	return block;
    }

    // (7:0) <Card title={"Лог"}>
    function create_default_slot$3(ctx) {
    	let each_1_anchor;
    	let each_value = /*coreMessages*/ ctx[0];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block$2(get_each_context$2(ctx, each_value, i));
    	}

    	const block = {
    		c: function create() {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			each_1_anchor = empty();
    		},
    		m: function mount(target, anchor) {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(target, anchor);
    			}

    			insert_dev(target, each_1_anchor, anchor);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*coreMessages*/ 1) {
    				each_value = /*coreMessages*/ ctx[0];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context$2(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    					} else {
    						each_blocks[i] = create_each_block$2(child_ctx);
    						each_blocks[i].c();
    						each_blocks[i].m(each_1_anchor.parentNode, each_1_anchor);
    					}
    				}

    				for (; i < each_blocks.length; i += 1) {
    					each_blocks[i].d(1);
    				}

    				each_blocks.length = each_value.length;
    			}
    		},
    		d: function destroy(detaching) {
    			destroy_each(each_blocks, detaching);
    			if (detaching) detach_dev(each_1_anchor);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot$3.name,
    		type: "slot",
    		source: "(7:0) <Card title={\\\"Лог\\\"}>",
    		ctx
    	});

    	return block;
    }

    function create_fragment$4(ctx) {
    	let card;
    	let current;

    	card = new Card({
    			props: {
    				title: "Лог",
    				$$slots: { default: [create_default_slot$3] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(card.$$.fragment);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			mount_component(card, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			const card_changes = {};

    			if (dirty & /*$$scope, coreMessages*/ 17) {
    				card_changes.$$scope = { dirty, ctx };
    			}

    			card.$set(card_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(card.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(card.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(card, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$4.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$4($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Log', slots, []);
    	let { coreMessages } = $$props;
    	const writable_props = ['coreMessages'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Log> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('coreMessages' in $$props) $$invalidate(0, coreMessages = $$props.coreMessages);
    	};

    	$$self.$capture_state = () => ({ Card, coreMessages });

    	$$self.$inject_state = $$props => {
    		if ('coreMessages' in $$props) $$invalidate(0, coreMessages = $$props.coreMessages);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [coreMessages];
    }

    class Log extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$4, create_fragment$4, safe_not_equal, { coreMessages: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Log",
    			options,
    			id: create_fragment$4.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*coreMessages*/ ctx[0] === undefined && !('coreMessages' in props)) {
    			console.warn("<Log> was created without expected prop 'coreMessages'");
    		}
    	}

    	get coreMessages() {
    		throw new Error("<Log>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set coreMessages(value) {
    		throw new Error("<Log>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\pages\List.svelte generated by Svelte v3.46.4 */

    const { console: console_1$1 } = globals;
    const file$3 = "src\\pages\\List.svelte";

    function get_each_context$1(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[11] = list[i];
    	child_ctx[13] = i;
    	return child_ctx;
    }

    // (41:8) {#each deviceList as device, i}
    function create_each_block$1(ctx) {
    	let tr;
    	let td0;
    	let t0_value = /*device*/ ctx[11].name + "";
    	let t0;
    	let t1;
    	let td1;
    	let a;
    	let t2_value = /*device*/ ctx[11].ip + "";
    	let t2;
    	let a_href_value;
    	let t3;
    	let td2;
    	let t4_value = /*device*/ ctx[11].id + "";
    	let t4;
    	let t5;
    	let td3;
    	let t6_value = (/*device*/ ctx[11].status ? "online" : "offline") + "";
    	let t6;
    	let td3_class_value;
    	let t7;
    	let td4;
    	let crossicon;
    	let current;

    	crossicon = new Cross({
    			props: { i: /*i*/ ctx[13], click: /*func*/ ctx[5] },
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			tr = element("tr");
    			td0 = element("td");
    			t0 = text(t0_value);
    			t1 = space();
    			td1 = element("td");
    			a = element("a");
    			t2 = text(t2_value);
    			t3 = space();
    			td2 = element("td");
    			t4 = text(t4_value);
    			t5 = space();
    			td3 = element("td");
    			t6 = text(t6_value);
    			t7 = space();
    			td4 = element("td");
    			create_component(crossicon.$$.fragment);
    			attr_dev(td0, "class", "tbl-bdy-lg ipt-lg w-full");
    			add_location(td0, file$3, 42, 12, 1214);
    			attr_dev(a, "href", a_href_value = "http://" + /*device*/ ctx[11].ip);
    			add_location(a, file$3, 43, 49, 1320);
    			attr_dev(td1, "class", "tbl-bdy-lg ipt-lg w-full");
    			add_location(td1, file$3, 43, 12, 1283);
    			attr_dev(td2, "class", "tbl-bdy-lg ipt-lg w-full");
    			add_location(td2, file$3, 44, 12, 1386);
    			attr_dev(td3, "class", td3_class_value = "tbl-bdy-lg ipt-lg w-full " + (/*device*/ ctx[11].status ? 'bg-green-50' : 'bg-red-50'));
    			add_location(td3, file$3, 45, 12, 1453);
    			attr_dev(td4, "class", "tbl-bdy-lg");
    			add_location(td4, file$3, 46, 12, 1593);
    			attr_dev(tr, "class", "txt-sz txt-pad");
    			add_location(tr, file$3, 41, 10, 1173);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, tr, anchor);
    			append_dev(tr, td0);
    			append_dev(td0, t0);
    			append_dev(tr, t1);
    			append_dev(tr, td1);
    			append_dev(td1, a);
    			append_dev(a, t2);
    			append_dev(tr, t3);
    			append_dev(tr, td2);
    			append_dev(td2, t4);
    			append_dev(tr, t5);
    			append_dev(tr, td3);
    			append_dev(td3, t6);
    			append_dev(tr, t7);
    			append_dev(tr, td4);
    			mount_component(crossicon, td4, null);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if ((!current || dirty & /*deviceList*/ 1) && t0_value !== (t0_value = /*device*/ ctx[11].name + "")) set_data_dev(t0, t0_value);
    			if ((!current || dirty & /*deviceList*/ 1) && t2_value !== (t2_value = /*device*/ ctx[11].ip + "")) set_data_dev(t2, t2_value);

    			if (!current || dirty & /*deviceList*/ 1 && a_href_value !== (a_href_value = "http://" + /*device*/ ctx[11].ip)) {
    				attr_dev(a, "href", a_href_value);
    			}

    			if ((!current || dirty & /*deviceList*/ 1) && t4_value !== (t4_value = /*device*/ ctx[11].id + "")) set_data_dev(t4, t4_value);
    			if ((!current || dirty & /*deviceList*/ 1) && t6_value !== (t6_value = (/*device*/ ctx[11].status ? "online" : "offline") + "")) set_data_dev(t6, t6_value);

    			if (!current || dirty & /*deviceList*/ 1 && td3_class_value !== (td3_class_value = "tbl-bdy-lg ipt-lg w-full " + (/*device*/ ctx[11].status ? 'bg-green-50' : 'bg-red-50'))) {
    				attr_dev(td3, "class", td3_class_value);
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(crossicon.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(crossicon.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(tr);
    			destroy_component(crossicon);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block$1.name,
    		type: "each",
    		source: "(41:8) {#each deviceList as device, i}",
    		ctx
    	});

    	return block;
    }

    // (50:8) {#if showInput}
    function create_if_block$1(ctx) {
    	let tr;
    	let td0;
    	let input0;
    	let t0;
    	let td1;
    	let input1;
    	let t1;
    	let td2;
    	let input2;
    	let t2;
    	let td3;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			tr = element("tr");
    			td0 = element("td");
    			input0 = element("input");
    			t0 = space();
    			td1 = element("td");
    			input1 = element("input");
    			t1 = space();
    			td2 = element("td");
    			input2 = element("input");
    			t2 = space();
    			td3 = element("td");
    			attr_dev(input0, "class", "ipt-lg w-full");
    			attr_dev(input0, "type", "text");
    			add_location(input0, file$3, 51, 35, 1815);
    			attr_dev(td0, "class", "tbl-bdy-lg");
    			add_location(td0, file$3, 51, 12, 1792);
    			attr_dev(input1, "class", "ipt-lg w-full");
    			attr_dev(input1, "type", "text");
    			add_location(input1, file$3, 52, 35, 1928);
    			attr_dev(td1, "class", "tbl-bdy-lg");
    			add_location(td1, file$3, 52, 12, 1905);
    			attr_dev(input2, "class", "ipt-lg w-full");
    			attr_dev(input2, "type", "text");
    			add_location(input2, file$3, 53, 35, 2039);
    			attr_dev(td2, "class", "tbl-bdy-lg");
    			add_location(td2, file$3, 53, 12, 2016);
    			attr_dev(td3, "class", "tbl-bdy-lg");
    			add_location(td3, file$3, 54, 12, 2127);
    			attr_dev(tr, "class", "txt-sz txt-pad");
    			add_location(tr, file$3, 50, 10, 1751);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, tr, anchor);
    			append_dev(tr, td0);
    			append_dev(td0, input0);
    			set_input_value(input0, /*newDevice*/ ctx[2].name);
    			append_dev(tr, t0);
    			append_dev(tr, td1);
    			append_dev(td1, input1);
    			set_input_value(input1, /*newDevice*/ ctx[2].ip);
    			append_dev(tr, t1);
    			append_dev(tr, td2);
    			append_dev(td2, input2);
    			set_input_value(input2, /*newDevice*/ ctx[2].id);
    			append_dev(tr, t2);
    			append_dev(tr, td3);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input0, "input", /*input0_input_handler*/ ctx[6]),
    					listen_dev(input1, "input", /*input1_input_handler*/ ctx[7]),
    					listen_dev(input2, "input", /*input2_input_handler*/ ctx[8])
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*newDevice*/ 4 && input0.value !== /*newDevice*/ ctx[2].name) {
    				set_input_value(input0, /*newDevice*/ ctx[2].name);
    			}

    			if (dirty & /*newDevice*/ 4 && input1.value !== /*newDevice*/ ctx[2].ip) {
    				set_input_value(input1, /*newDevice*/ ctx[2].ip);
    			}

    			if (dirty & /*newDevice*/ 4 && input2.value !== /*newDevice*/ ctx[2].id) {
    				set_input_value(input2, /*newDevice*/ ctx[2].id);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(tr);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$1.name,
    		type: "if",
    		source: "(50:8) {#if showInput}",
    		ctx
    	});

    	return block;
    }

    // (29:2) <Card title={"Список устройств"}>
    function create_default_slot_1$1(ctx) {
    	let table;
    	let thead;
    	let tr;
    	let th0;
    	let t1;
    	let th1;
    	let t3;
    	let th2;
    	let t5;
    	let th3;
    	let t7;
    	let th4;
    	let t8;
    	let tbody;
    	let t9;
    	let t10;
    	let button;

    	let t11_value = (/*showInput*/ ctx[1]
    	? "Сохранить"
    	: "Добавить устройство") + "";

    	let t11;
    	let current;
    	let mounted;
    	let dispose;
    	let each_value = /*deviceList*/ ctx[0];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block$1(get_each_context$1(ctx, each_value, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	let if_block = /*showInput*/ ctx[1] && create_if_block$1(ctx);

    	const block = {
    		c: function create() {
    			table = element("table");
    			thead = element("thead");
    			tr = element("tr");
    			th0 = element("th");
    			th0.textContent = "Название устройства";
    			t1 = space();
    			th1 = element("th");
    			th1.textContent = "IP адрес";
    			t3 = space();
    			th2 = element("th");
    			th2.textContent = "Идентификатор";
    			t5 = space();
    			th3 = element("th");
    			th3.textContent = "Состояние";
    			t7 = space();
    			th4 = element("th");
    			t8 = space();
    			tbody = element("tbody");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			t9 = space();
    			if (if_block) if_block.c();
    			t10 = space();
    			button = element("button");
    			t11 = text(t11_value);
    			attr_dev(th0, "class", "tbl-hd");
    			add_location(th0, file$3, 32, 10, 839);
    			attr_dev(th1, "class", "tbl-hd");
    			add_location(th1, file$3, 33, 10, 894);
    			attr_dev(th2, "class", "tbl-hd");
    			add_location(th2, file$3, 34, 10, 938);
    			attr_dev(th3, "class", "tbl-hd");
    			add_location(th3, file$3, 35, 10, 987);
    			attr_dev(th4, "class", "tbl-hd w-7");
    			add_location(th4, file$3, 36, 10, 1032);
    			attr_dev(tr, "class", "txt-sz txt-pad");
    			add_location(tr, file$3, 31, 8, 800);
    			attr_dev(thead, "class", "bg-gray-100");
    			add_location(thead, file$3, 30, 6, 763);
    			attr_dev(tbody, "class", "bg-white");
    			add_location(tbody, file$3, 39, 6, 1096);
    			attr_dev(table, "class", "tbl");
    			add_location(table, file$3, 29, 4, 736);
    			attr_dev(button, "class", "btn-lg");
    			add_location(button, file$3, 59, 4, 2220);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, table, anchor);
    			append_dev(table, thead);
    			append_dev(thead, tr);
    			append_dev(tr, th0);
    			append_dev(tr, t1);
    			append_dev(tr, th1);
    			append_dev(tr, t3);
    			append_dev(tr, th2);
    			append_dev(tr, t5);
    			append_dev(tr, th3);
    			append_dev(tr, t7);
    			append_dev(tr, th4);
    			append_dev(table, t8);
    			append_dev(table, tbody);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(tbody, null);
    			}

    			append_dev(tbody, t9);
    			if (if_block) if_block.m(tbody, null);
    			insert_dev(target, t10, anchor);
    			insert_dev(target, button, anchor);
    			append_dev(button, t11);
    			current = true;

    			if (!mounted) {
    				dispose = listen_dev(button, "click", /*click_handler*/ ctx[9], false, false, false);
    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*deleteLineFromDevlist, deviceList*/ 17) {
    				each_value = /*deviceList*/ ctx[0];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context$1(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block$1(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(tbody, t9);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}

    			if (/*showInput*/ ctx[1]) {
    				if (if_block) {
    					if_block.p(ctx, dirty);
    				} else {
    					if_block = create_if_block$1(ctx);
    					if_block.c();
    					if_block.m(tbody, null);
    				}
    			} else if (if_block) {
    				if_block.d(1);
    				if_block = null;
    			}

    			if ((!current || dirty & /*showInput*/ 2) && t11_value !== (t11_value = (/*showInput*/ ctx[1]
    			? "Сохранить"
    			: "Добавить устройство") + "")) set_data_dev(t11, t11_value);
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(table);
    			destroy_each(each_blocks, detaching);
    			if (if_block) if_block.d();
    			if (detaching) detach_dev(t10);
    			if (detaching) detach_dev(button);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_1$1.name,
    		type: "slot",
    		source: "(29:2) <Card title={\\\"Список устройств\\\"}>",
    		ctx
    	});

    	return block;
    }

    // (62:2) <Alarm>
    function create_default_slot$2(ctx) {
    	let p;

    	const block = {
    		c: function create() {
    			p = element("p");
    			p.textContent = "Список устройств будет обновляться автоматически. Подключенные к одному роутеру устройства будут появляться в списке в течении двух минут. Ручное добавление сделано в целях проверки для разработчика.";
    			add_location(p, file$3, 62, 4, 2388);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, p, anchor);
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(p);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot$2.name,
    		type: "slot",
    		source: "(62:2) <Alarm>",
    		ctx
    	});

    	return block;
    }

    function create_fragment$3(ctx) {
    	let div;
    	let card;
    	let t;
    	let alarm;
    	let current;

    	card = new Card({
    			props: {
    				title: "Список устройств",
    				$$slots: { default: [create_default_slot_1$1] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	alarm = new Alarm({
    			props: {
    				$$slots: { default: [create_default_slot$2] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			div = element("div");
    			create_component(card.$$.fragment);
    			t = space();
    			create_component(alarm.$$.fragment);
    			attr_dev(div, "class", "grd-1col1");
    			add_location(div, file$3, 27, 0, 670);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			mount_component(card, div, null);
    			append_dev(div, t);
    			mount_component(alarm, div, null);
    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			const card_changes = {};

    			if (dirty & /*$$scope, showInput, devListSave, newDevice, deviceList*/ 16399) {
    				card_changes.$$scope = { dirty, ctx };
    			}

    			card.$set(card_changes);
    			const alarm_changes = {};

    			if (dirty & /*$$scope*/ 16384) {
    				alarm_changes.$$scope = { dirty, ctx };
    			}

    			alarm.$set(alarm_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(card.$$.fragment, local);
    			transition_in(alarm.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(card.$$.fragment, local);
    			transition_out(alarm.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			destroy_component(card);
    			destroy_component(alarm);
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

    function instance$3($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('List', slots, []);
    	let { deviceList } = $$props;
    	let { showInput } = $$props;
    	let { newDevice = {} } = $$props;

    	let { devListSave = () => {
    		
    	} } = $$props;

    	let debug = true;

    	function deleteLineFromDevlist(num) {
    		for (let i = 0; i < deviceList.length; i++) {
    			if (num === i) {
    				deviceList.splice(i, 1);
    				$$invalidate(0, deviceList);
    				if (debug) console.log("[i]", "item " + num + " deleted from dev list");
    				break;
    			}
    		}
    	}

    	const writable_props = ['deviceList', 'showInput', 'newDevice', 'devListSave'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console_1$1.warn(`<List> was created with unknown prop '${key}'`);
    	});

    	const func = i => deleteLineFromDevlist(i);

    	function input0_input_handler() {
    		newDevice.name = this.value;
    		$$invalidate(2, newDevice);
    	}

    	function input1_input_handler() {
    		newDevice.ip = this.value;
    		$$invalidate(2, newDevice);
    	}

    	function input2_input_handler() {
    		newDevice.id = this.value;
    		$$invalidate(2, newDevice);
    	}

    	const click_handler = () => ($$invalidate(1, showInput = !showInput), devListSave());

    	$$self.$$set = $$props => {
    		if ('deviceList' in $$props) $$invalidate(0, deviceList = $$props.deviceList);
    		if ('showInput' in $$props) $$invalidate(1, showInput = $$props.showInput);
    		if ('newDevice' in $$props) $$invalidate(2, newDevice = $$props.newDevice);
    		if ('devListSave' in $$props) $$invalidate(3, devListSave = $$props.devListSave);
    	};

    	$$self.$capture_state = () => ({
    		Card,
    		Alarm,
    		CrossIcon: Cross,
    		OpenIcon: Open,
    		deviceList,
    		showInput,
    		newDevice,
    		devListSave,
    		debug,
    		deleteLineFromDevlist
    	});

    	$$self.$inject_state = $$props => {
    		if ('deviceList' in $$props) $$invalidate(0, deviceList = $$props.deviceList);
    		if ('showInput' in $$props) $$invalidate(1, showInput = $$props.showInput);
    		if ('newDevice' in $$props) $$invalidate(2, newDevice = $$props.newDevice);
    		if ('devListSave' in $$props) $$invalidate(3, devListSave = $$props.devListSave);
    		if ('debug' in $$props) debug = $$props.debug;
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		deviceList,
    		showInput,
    		newDevice,
    		devListSave,
    		deleteLineFromDevlist,
    		func,
    		input0_input_handler,
    		input1_input_handler,
    		input2_input_handler,
    		click_handler
    	];
    }

    class List extends SvelteComponentDev {
    	constructor(options) {
    		super(options);

    		init(this, options, instance$3, create_fragment$3, safe_not_equal, {
    			deviceList: 0,
    			showInput: 1,
    			newDevice: 2,
    			devListSave: 3
    		});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "List",
    			options,
    			id: create_fragment$3.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*deviceList*/ ctx[0] === undefined && !('deviceList' in props)) {
    			console_1$1.warn("<List> was created without expected prop 'deviceList'");
    		}

    		if (/*showInput*/ ctx[1] === undefined && !('showInput' in props)) {
    			console_1$1.warn("<List> was created without expected prop 'showInput'");
    		}
    	}

    	get deviceList() {
    		throw new Error("<List>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set deviceList(value) {
    		throw new Error("<List>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get showInput() {
    		throw new Error("<List>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set showInput(value) {
    		throw new Error("<List>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get newDevice() {
    		throw new Error("<List>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set newDevice(value) {
    		throw new Error("<List>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get devListSave() {
    		throw new Error("<List>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set devListSave(value) {
    		throw new Error("<List>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\pages\About.svelte generated by Svelte v3.46.4 */
    const file$2 = "src\\pages\\About.svelte";

    // (12:0) <Card title="Редактор JSON">
    function create_default_slot$1(ctx) {
    	let textarea;
    	let textarea_value_value;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			textarea = element("textarea");
    			attr_dev(textarea, "rows", "10");
    			attr_dev(textarea, "class", "ipt-big w-full");
    			attr_dev(textarea, "id", "text1");
    			textarea.value = textarea_value_value = /*syntaxHighlight*/ ctx[3](JSON.stringify(/*layoutJson*/ ctx[1]));
    			add_location(textarea, file$2, 12, 2, 320);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, textarea, anchor);

    			if (!mounted) {
    				dispose = listen_dev(
    					textarea,
    					"input",
    					function () {
    						if (is_function(/*wigetsUpdate*/ ctx[0])) /*wigetsUpdate*/ ctx[0].apply(this, arguments);
    					},
    					false,
    					false,
    					false
    				);

    				mounted = true;
    			}
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;

    			if (dirty & /*syntaxHighlight, layoutJson*/ 10 && textarea_value_value !== (textarea_value_value = /*syntaxHighlight*/ ctx[3](JSON.stringify(/*layoutJson*/ ctx[1])))) {
    				prop_dev(textarea, "value", textarea_value_value);
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(textarea);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot$1.name,
    		type: "slot",
    		source: "(12:0) <Card title=\\\"Редактор JSON\\\">",
    		ctx
    	});

    	return block;
    }

    function create_fragment$2(ctx) {
    	let button;
    	let t1;
    	let card;
    	let current;
    	let mounted;
    	let dispose;

    	card = new Card({
    			props: {
    				title: "Редактор JSON",
    				$$slots: { default: [create_default_slot$1] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			button = element("button");
    			button.textContent = "Toggle modal";
    			t1 = space();
    			create_component(card.$$.fragment);
    			attr_dev(button, "type", "button");
    			add_location(button, file$2, 10, 0, 212);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, button, anchor);
    			insert_dev(target, t1, anchor);
    			mount_component(card, target, anchor);
    			current = true;

    			if (!mounted) {
    				dispose = listen_dev(button, "click", /*click_handler*/ ctx[4], false, false, false);
    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			const card_changes = {};

    			if (dirty & /*$$scope, syntaxHighlight, layoutJson, wigetsUpdate*/ 43) {
    				card_changes.$$scope = { dirty, ctx };
    			}

    			card.$set(card_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(card.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(card.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(button);
    			if (detaching) detach_dev(t1);
    			destroy_component(card, detaching);
    			mounted = false;
    			dispose();
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
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('About', slots, []);
    	let { wigetsUpdate } = $$props;
    	let { layoutJson } = $$props;

    	let { showModal = () => {
    		
    	} } = $$props;

    	let { syntaxHighlight = json => {
    		
    	} } = $$props;

    	const writable_props = ['wigetsUpdate', 'layoutJson', 'showModal', 'syntaxHighlight'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<About> was created with unknown prop '${key}'`);
    	});

    	const click_handler = () => showModal();

    	$$self.$$set = $$props => {
    		if ('wigetsUpdate' in $$props) $$invalidate(0, wigetsUpdate = $$props.wigetsUpdate);
    		if ('layoutJson' in $$props) $$invalidate(1, layoutJson = $$props.layoutJson);
    		if ('showModal' in $$props) $$invalidate(2, showModal = $$props.showModal);
    		if ('syntaxHighlight' in $$props) $$invalidate(3, syntaxHighlight = $$props.syntaxHighlight);
    	};

    	$$self.$capture_state = () => ({
    		Card,
    		wigetsUpdate,
    		layoutJson,
    		showModal,
    		syntaxHighlight
    	});

    	$$self.$inject_state = $$props => {
    		if ('wigetsUpdate' in $$props) $$invalidate(0, wigetsUpdate = $$props.wigetsUpdate);
    		if ('layoutJson' in $$props) $$invalidate(1, layoutJson = $$props.layoutJson);
    		if ('showModal' in $$props) $$invalidate(2, showModal = $$props.showModal);
    		if ('syntaxHighlight' in $$props) $$invalidate(3, syntaxHighlight = $$props.syntaxHighlight);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [wigetsUpdate, layoutJson, showModal, syntaxHighlight, click_handler];
    }

    class About extends SvelteComponentDev {
    	constructor(options) {
    		super(options);

    		init(this, options, instance$2, create_fragment$2, safe_not_equal, {
    			wigetsUpdate: 0,
    			layoutJson: 1,
    			showModal: 2,
    			syntaxHighlight: 3
    		});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "About",
    			options,
    			id: create_fragment$2.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*wigetsUpdate*/ ctx[0] === undefined && !('wigetsUpdate' in props)) {
    			console.warn("<About> was created without expected prop 'wigetsUpdate'");
    		}

    		if (/*layoutJson*/ ctx[1] === undefined && !('layoutJson' in props)) {
    			console.warn("<About> was created without expected prop 'layoutJson'");
    		}
    	}

    	get wigetsUpdate() {
    		throw new Error("<About>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set wigetsUpdate(value) {
    		throw new Error("<About>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get layoutJson() {
    		throw new Error("<About>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set layoutJson(value) {
    		throw new Error("<About>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get showModal() {
    		throw new Error("<About>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set showModal(value) {
    		throw new Error("<About>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get syntaxHighlight() {
    		throw new Error("<About>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set syntaxHighlight(value) {
    		throw new Error("<About>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\svg\Cloud.svelte generated by Svelte v3.46.4 */

    const file$1 = "src\\svg\\Cloud.svelte";

    function create_fragment$1(ctx) {
    	let svg;
    	let path0;
    	let path1;
    	let svg_class_value;

    	const block = {
    		c: function create() {
    			svg = svg_element("svg");
    			path0 = svg_element("path");
    			path1 = svg_element("path");
    			attr_dev(path0, "stroke", "none");
    			attr_dev(path0, "d", "M0 0h24v24H0z");
    			add_location(path0, file$1, 5, 2, 218);
    			attr_dev(path1, "d", "M7 18a4.6 4.4 0 0 1 0 -9h0a5 4.5 0 0 1 11 2h1a3.5 3.5 0 0 1 0 7h-12");
    			add_location(path1, file$1, 6, 2, 262);
    			attr_dev(svg, "class", svg_class_value = "h-8 w-8 " + /*color*/ ctx[0]);
    			attr_dev(svg, "width", "24");
    			attr_dev(svg, "height", "24");
    			attr_dev(svg, "viewBox", "0 0 24 24");
    			attr_dev(svg, "stroke-width", "2");
    			attr_dev(svg, "stroke", "currentColor");
    			attr_dev(svg, "fill", "none");
    			attr_dev(svg, "stroke-linecap", "round");
    			attr_dev(svg, "stroke-linejoin", "round");
    			add_location(svg, file$1, 4, 0, 44);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, svg, anchor);
    			append_dev(svg, path0);
    			append_dev(svg, path1);
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*color*/ 1 && svg_class_value !== (svg_class_value = "h-8 w-8 " + /*color*/ ctx[0])) {
    				attr_dev(svg, "class", svg_class_value);
    			}
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(svg);
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
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Cloud', slots, []);
    	let { color } = $$props;
    	const writable_props = ['color'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Cloud> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('color' in $$props) $$invalidate(0, color = $$props.color);
    	};

    	$$self.$capture_state = () => ({ color });

    	$$self.$inject_state = $$props => {
    		if ('color' in $$props) $$invalidate(0, color = $$props.color);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [color];
    }

    class Cloud extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$1, create_fragment$1, safe_not_equal, { color: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Cloud",
    			options,
    			id: create_fragment$1.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*color*/ ctx[0] === undefined && !('color' in props)) {
    			console.warn("<Cloud> was created without expected prop 'color'");
    		}
    	}

    	get color() {
    		throw new Error("<Cloud>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set color(value) {
    		throw new Error("<Cloud>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\App.svelte generated by Svelte v3.46.4 */

    const { Object: Object_1, console: console_1 } = globals;
    const file = "src\\App.svelte";

    function get_each_context(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[90] = list[i];
    	return child_ctx;
    }

    // (760:10) {#each deviceList as device}
    function create_each_block(ctx) {
    	let option;
    	let t0_value = /*device*/ ctx[90].name + "";
    	let t0;
    	let t1;
    	let option_value_value;

    	const block = {
    		c: function create() {
    			option = element("option");
    			t0 = text(t0_value);
    			t1 = space();
    			option.__value = option_value_value = /*device*/ ctx[90].ws;
    			option.value = option.__value;
    			add_location(option, file, 760, 12, 24626);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, option, anchor);
    			append_dev(option, t0);
    			append_dev(option, t1);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty[0] & /*deviceList*/ 16384 && t0_value !== (t0_value = /*device*/ ctx[90].name + "")) set_data_dev(t0, t0_value);

    			if (dirty[0] & /*deviceList*/ 16384 && option_value_value !== (option_value_value = /*device*/ ctx[90].ws)) {
    				prop_dev(option, "__value", option_value_value);
    				option.value = option.__value;
    			}
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(option);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block.name,
    		type: "each",
    		source: "(760:10) {#each deviceList as device}",
    		ctx
    	});

    	return block;
    }

    // (809:8) {:else}
    function create_else_block(ctx) {
    	let route0;
    	let t0;
    	let route1;
    	let t1;
    	let route2;
    	let t2;
    	let route3;
    	let t3;
    	let route4;
    	let t4;
    	let route5;
    	let current;

    	route0 = new Route({
    			props: {
    				path: "/",
    				$$slots: { default: [create_default_slot_6] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	route1 = new Route({
    			props: {
    				path: "/config",
    				$$slots: { default: [create_default_slot_5] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	route2 = new Route({
    			props: {
    				path: "/connection",
    				$$slots: { default: [create_default_slot_4] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	route3 = new Route({
    			props: {
    				path: "/utilities",
    				$$slots: { default: [create_default_slot_3] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	route4 = new Route({
    			props: {
    				path: "/log",
    				$$slots: { default: [create_default_slot_2] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	route5 = new Route({
    			props: {
    				path: "/about",
    				$$slots: { default: [create_default_slot_1] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(route0.$$.fragment);
    			t0 = space();
    			create_component(route1.$$.fragment);
    			t1 = space();
    			create_component(route2.$$.fragment);
    			t2 = space();
    			create_component(route3.$$.fragment);
    			t3 = space();
    			create_component(route4.$$.fragment);
    			t4 = space();
    			create_component(route5.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(route0, target, anchor);
    			insert_dev(target, t0, anchor);
    			mount_component(route1, target, anchor);
    			insert_dev(target, t1, anchor);
    			mount_component(route2, target, anchor);
    			insert_dev(target, t2, anchor);
    			mount_component(route3, target, anchor);
    			insert_dev(target, t3, anchor);
    			mount_component(route4, target, anchor);
    			insert_dev(target, t4, anchor);
    			mount_component(route5, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const route0_changes = {};

    			if (dirty[0] & /*layoutJson, pages*/ 136 | dirty[3] & /*$$scope*/ 1) {
    				route0_changes.$$scope = { dirty, ctx };
    			}

    			route0.$set(route0_changes);
    			const route1_changes = {};

    			if (dirty[0] & /*configJson, widgetsJson, itemsJson*/ 112 | dirty[3] & /*$$scope*/ 1) {
    				route1_changes.$$scope = { dirty, ctx };
    			}

    			route1.$set(route1_changes);
    			const route2_changes = {};

    			if (dirty[0] & /*settingsJson, ssidJson*/ 768 | dirty[3] & /*$$scope*/ 1) {
    				route2_changes.$$scope = { dirty, ctx };
    			}

    			route2.$set(route2_changes);
    			const route3_changes = {};

    			if (dirty[3] & /*$$scope*/ 1) {
    				route3_changes.$$scope = { dirty, ctx };
    			}

    			route3.$set(route3_changes);
    			const route4_changes = {};

    			if (dirty[0] & /*coreMessages*/ 8192 | dirty[3] & /*$$scope*/ 1) {
    				route4_changes.$$scope = { dirty, ctx };
    			}

    			route4.$set(route4_changes);
    			const route5_changes = {};

    			if (dirty[0] & /*layoutJson*/ 128 | dirty[3] & /*$$scope*/ 1) {
    				route5_changes.$$scope = { dirty, ctx };
    			}

    			route5.$set(route5_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(route0.$$.fragment, local);
    			transition_in(route1.$$.fragment, local);
    			transition_in(route2.$$.fragment, local);
    			transition_in(route3.$$.fragment, local);
    			transition_in(route4.$$.fragment, local);
    			transition_in(route5.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(route0.$$.fragment, local);
    			transition_out(route1.$$.fragment, local);
    			transition_out(route2.$$.fragment, local);
    			transition_out(route3.$$.fragment, local);
    			transition_out(route4.$$.fragment, local);
    			transition_out(route5.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(route0, detaching);
    			if (detaching) detach_dev(t0);
    			destroy_component(route1, detaching);
    			if (detaching) detach_dev(t1);
    			destroy_component(route2, detaching);
    			if (detaching) detach_dev(t2);
    			destroy_component(route3, detaching);
    			if (detaching) detach_dev(t3);
    			destroy_component(route4, detaching);
    			if (detaching) detach_dev(t4);
    			destroy_component(route5, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_else_block.name,
    		type: "else",
    		source: "(809:8) {:else}",
    		ctx
    	});

    	return block;
    }

    // (807:8) {#if !socketConnected}
    function create_if_block(ctx) {
    	let alarm;
    	let current;

    	alarm = new Alarm({
    			props: { title: "Нет соединения" },
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(alarm.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(alarm, target, anchor);
    			current = true;
    		},
    		p: noop,
    		i: function intro(local) {
    			if (current) return;
    			transition_in(alarm.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(alarm.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(alarm, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block.name,
    		type: "if",
    		source: "(807:8) {#if !socketConnected}",
    		ctx
    	});

    	return block;
    }

    // (810:10) <Route path="/">
    function create_default_slot_6(ctx) {
    	let dashboardpage;
    	let current;

    	dashboardpage = new Dashboard({
    			props: {
    				layoutJson: /*layoutJson*/ ctx[7],
    				pages: /*pages*/ ctx[3],
    				wsPush: /*func*/ ctx[30]
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(dashboardpage.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(dashboardpage, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const dashboardpage_changes = {};
    			if (dirty[0] & /*layoutJson*/ 128) dashboardpage_changes.layoutJson = /*layoutJson*/ ctx[7];
    			if (dirty[0] & /*pages*/ 8) dashboardpage_changes.pages = /*pages*/ ctx[3];
    			dashboardpage.$set(dashboardpage_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(dashboardpage.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(dashboardpage.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(dashboardpage, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_6.name,
    		type: "slot",
    		source: "(810:10) <Route path=\\\"/\\\">",
    		ctx
    	});

    	return block;
    }

    // (813:10) <Route path="/config">
    function create_default_slot_5(ctx) {
    	let configpage;
    	let current;

    	configpage = new Config({
    			props: {
    				configJson: /*configJson*/ ctx[4],
    				widgetsJson: /*widgetsJson*/ ctx[5],
    				itemsJson: /*itemsJson*/ ctx[6],
    				saveConfig: /*func_1*/ ctx[31]
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(configpage.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(configpage, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const configpage_changes = {};
    			if (dirty[0] & /*configJson*/ 16) configpage_changes.configJson = /*configJson*/ ctx[4];
    			if (dirty[0] & /*widgetsJson*/ 32) configpage_changes.widgetsJson = /*widgetsJson*/ ctx[5];
    			if (dirty[0] & /*itemsJson*/ 64) configpage_changes.itemsJson = /*itemsJson*/ ctx[6];
    			configpage.$set(configpage_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(configpage.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(configpage.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(configpage, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_5.name,
    		type: "slot",
    		source: "(813:10) <Route path=\\\"/config\\\">",
    		ctx
    	});

    	return block;
    }

    // (816:10) <Route path="/connection">
    function create_default_slot_4(ctx) {
    	let connectionpage;
    	let current;

    	connectionpage = new Connection({
    			props: {
    				settingsJson: /*settingsJson*/ ctx[8],
    				ssidJson: /*ssidJson*/ ctx[9],
    				ssidDropdownClick: /*func_2*/ ctx[32],
    				saveSettings: /*func_3*/ ctx[33]
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(connectionpage.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(connectionpage, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const connectionpage_changes = {};
    			if (dirty[0] & /*settingsJson*/ 256) connectionpage_changes.settingsJson = /*settingsJson*/ ctx[8];
    			if (dirty[0] & /*ssidJson*/ 512) connectionpage_changes.ssidJson = /*ssidJson*/ ctx[9];
    			connectionpage.$set(connectionpage_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(connectionpage.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(connectionpage.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(connectionpage, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_4.name,
    		type: "slot",
    		source: "(816:10) <Route path=\\\"/connection\\\">",
    		ctx
    	});

    	return block;
    }

    // (819:10) <Route path="/utilities">
    function create_default_slot_3(ctx) {
    	let utilitiespage;
    	let current;
    	utilitiespage = new Utilities({ $$inline: true });

    	const block = {
    		c: function create() {
    			create_component(utilitiespage.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(utilitiespage, target, anchor);
    			current = true;
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(utilitiespage.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(utilitiespage.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(utilitiespage, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_3.name,
    		type: "slot",
    		source: "(819:10) <Route path=\\\"/utilities\\\">",
    		ctx
    	});

    	return block;
    }

    // (822:10) <Route path="/log">
    function create_default_slot_2(ctx) {
    	let logpage;
    	let current;

    	logpage = new Log({
    			props: { coreMessages: /*coreMessages*/ ctx[13] },
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(logpage.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(logpage, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const logpage_changes = {};
    			if (dirty[0] & /*coreMessages*/ 8192) logpage_changes.coreMessages = /*coreMessages*/ ctx[13];
    			logpage.$set(logpage_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(logpage.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(logpage.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(logpage, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_2.name,
    		type: "slot",
    		source: "(822:10) <Route path=\\\"/log\\\">",
    		ctx
    	});

    	return block;
    }

    // (825:10) <Route path="/about">
    function create_default_slot_1(ctx) {
    	let aboutpage;
    	let current;

    	aboutpage = new About({
    			props: {
    				wigetsUpdate: /*wigetsUpdate*/ ctx[19],
    				layoutJson: /*layoutJson*/ ctx[7],
    				showModal: /*func_4*/ ctx[34],
    				syntaxHighlight: /*func_5*/ ctx[35]
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(aboutpage.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(aboutpage, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const aboutpage_changes = {};
    			if (dirty[0] & /*layoutJson*/ 128) aboutpage_changes.layoutJson = /*layoutJson*/ ctx[7];
    			aboutpage.$set(aboutpage_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(aboutpage.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(aboutpage.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(aboutpage, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot_1.name,
    		type: "slot",
    		source: "(825:10) <Route path=\\\"/about\\\">",
    		ctx
    	});

    	return block;
    }

    // (829:8) <Route path="/list">
    function create_default_slot(ctx) {
    	let listpage;
    	let current;

    	listpage = new List({
    			props: {
    				deviceList: /*deviceList*/ ctx[14],
    				showInput: /*showInput*/ ctx[15],
    				devListSave: /*func_6*/ ctx[36],
    				newDevice: /*newDevice*/ ctx[12]
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(listpage.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(listpage, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const listpage_changes = {};
    			if (dirty[0] & /*deviceList*/ 16384) listpage_changes.deviceList = /*deviceList*/ ctx[14];
    			if (dirty[0] & /*newDevice*/ 4096) listpage_changes.newDevice = /*newDevice*/ ctx[12];
    			listpage.$set(listpage_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(listpage.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(listpage.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(listpage, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot.name,
    		type: "slot",
    		source: "(829:8) <Route path=\\\"/list\\\">",
    		ctx
    	});

    	return block;
    }

    function create_fragment(ctx) {
    	let div5;
    	let modal;
    	let t0;
    	let header;
    	let div2;
    	let div0;
    	let select;
    	let t1;
    	let div1;
    	let cloudicon;
    	let t2;
    	let nav;
    	let input;
    	let t3;
    	let label;
    	let span;
    	let t4;
    	let ul0;
    	let li0;
    	let a0;
    	let t6;
    	let li1;
    	let a1;
    	let t8;
    	let li2;
    	let a2;
    	let t10;
    	let li3;
    	let a3;
    	let t12;
    	let li4;
    	let a4;
    	let t14;
    	let li5;
    	let a5;
    	let t16;
    	let li6;
    	let a6;
    	let t18;
    	let main;
    	let ul1;
    	let div3;
    	let current_block_type_index;
    	let if_block;
    	let t19;
    	let route;
    	let main_class_value;
    	let t20;
    	let footer;
    	let div4;
    	let current;
    	let mounted;
    	let dispose;

    	modal = new Modal({
    			props: { show: /*showModalFlag*/ ctx[2] },
    			$$inline: true
    		});

    	let each_value = /*deviceList*/ ctx[14];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block(get_each_context(ctx, each_value, i));
    	}

    	cloudicon = new Cloud({
    			props: {
    				color: /*socketConnected*/ ctx[10] === true
    				? "text-green-500"
    				: "text-red-500"
    			},
    			$$inline: true
    		});

    	const if_block_creators = [create_if_block, create_else_block];
    	const if_blocks = [];

    	function select_block_type(ctx, dirty) {
    		if (!/*socketConnected*/ ctx[10]) return 0;
    		return 1;
    	}

    	current_block_type_index = select_block_type(ctx);
    	if_block = if_blocks[current_block_type_index] = if_block_creators[current_block_type_index](ctx);

    	route = new Route({
    			props: {
    				path: "/list",
    				$$slots: { default: [create_default_slot] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			div5 = element("div");
    			create_component(modal.$$.fragment);
    			t0 = space();
    			header = element("header");
    			div2 = element("div");
    			div0 = element("div");
    			select = element("select");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			t1 = space();
    			div1 = element("div");
    			create_component(cloudicon.$$.fragment);
    			t2 = space();
    			nav = element("nav");
    			input = element("input");
    			t3 = space();
    			label = element("label");
    			span = element("span");
    			t4 = space();
    			ul0 = element("ul");
    			li0 = element("li");
    			a0 = element("a");
    			a0.textContent = `${"Управление"}`;
    			t6 = space();
    			li1 = element("li");
    			a1 = element("a");
    			a1.textContent = `${"Конфигуратор"}`;
    			t8 = space();
    			li2 = element("li");
    			a2 = element("a");
    			a2.textContent = `${"Подключение"}`;
    			t10 = space();
    			li3 = element("li");
    			a3 = element("a");
    			a3.textContent = `${"Утилиты"}`;
    			t12 = space();
    			li4 = element("li");
    			a4 = element("a");
    			a4.textContent = `${"Лог"}`;
    			t14 = space();
    			li5 = element("li");
    			a5 = element("a");
    			a5.textContent = `${"Устройства"}`;
    			t16 = space();
    			li6 = element("li");
    			a6 = element("a");
    			a6.textContent = `${"О проекте"}`;
    			t18 = space();
    			main = element("main");
    			ul1 = element("ul");
    			div3 = element("div");
    			if_block.c();
    			t19 = space();
    			create_component(route.$$.fragment);
    			t20 = space();
    			footer = element("footer");
    			div4 = element("div");
    			div4.textContent = "Developed by Dmitry Borisenko";
    			attr_dev(select, "class", "border border-indigo-500 border-4");
    			if (/*selectedWs*/ ctx[11] === void 0) add_render_callback(() => /*select_change_handler*/ ctx[26].call(select));
    			add_location(select, file, 758, 8, 24456);
    			attr_dev(div0, "class", "px-15 py-1 z-50 ");
    			add_location(div0, file, 757, 6, 24416);
    			attr_dev(div1, "class", "pl-4 pr-4 py-1");
    			add_location(div1, file, 766, 6, 24764);
    			attr_dev(div2, "class", "flex content-center items-center justify-end");
    			add_location(div2, file, 756, 4, 24350);
    			attr_dev(header, "class", "h-10 w-full bg-gray-100 overflow-auto shadow-md");
    			add_location(header, file, 755, 2, 24280);
    			attr_dev(input, "id", "menu__toggle");
    			attr_dev(input, "type", "checkbox");
    			add_location(input, file, 773, 4, 24953);
    			add_location(span, file, 775, 6, 25104);
    			attr_dev(label, "class", "menu__btn");
    			attr_dev(label, "for", "menu__toggle");
    			add_location(label, file, 774, 4, 25052);
    			attr_dev(a0, "class", "menu__item");
    			attr_dev(a0, "href", "/");
    			add_location(a0, file, 780, 8, 25178);
    			add_location(li0, file, 779, 6, 25164);
    			attr_dev(a1, "class", "menu__item");
    			attr_dev(a1, "href", "/config");
    			add_location(a1, file, 783, 8, 25262);
    			add_location(li1, file, 782, 6, 25248);
    			attr_dev(a2, "class", "menu__item");
    			attr_dev(a2, "href", "/connection");
    			add_location(a2, file, 786, 8, 25354);
    			add_location(li2, file, 785, 6, 25340);
    			attr_dev(a3, "class", "menu__item");
    			attr_dev(a3, "href", "/utilities");
    			add_location(a3, file, 789, 8, 25449);
    			add_location(li3, file, 788, 6, 25435);
    			attr_dev(a4, "class", "menu__item");
    			attr_dev(a4, "href", "/log");
    			add_location(a4, file, 792, 8, 25539);
    			add_location(li4, file, 791, 6, 25525);
    			attr_dev(a5, "class", "menu__item");
    			attr_dev(a5, "href", "/list");
    			add_location(a5, file, 795, 8, 25619);
    			add_location(li5, file, 794, 6, 25605);
    			attr_dev(a6, "class", "menu__item");
    			attr_dev(a6, "href", "/about");
    			add_location(a6, file, 798, 8, 25707);
    			add_location(li6, file, 797, 6, 25693);
    			attr_dev(ul0, "class", "menu__box");
    			add_location(ul0, file, 778, 4, 25134);
    			attr_dev(nav, "class", "flex");
    			add_location(nav, file, 772, 2, 24929);
    			attr_dev(div3, "class", "bg-cover pt-0 px-4");
    			add_location(div3, file, 805, 6, 25931);
    			attr_dev(ul1, "class", "menu__main");
    			add_location(ul1, file, 804, 4, 25900);

    			attr_dev(main, "class", main_class_value = "flex-1 overflow-y-auto p-0 " + (/*opened*/ ctx[0] === true && !/*preventMove*/ ctx[1]
    			? 'ml-36'
    			: 'ml-0'));

    			add_location(main, file, 803, 2, 25800);
    			attr_dev(div4, "class", "flex justify-center content-center text-xxs text-gray-500");
    			add_location(div4, file, 836, 4, 27336);
    			attr_dev(footer, "class", "h-4 bg-gray-100 border-gray-200 shadow-lg");
    			add_location(footer, file, 835, 2, 27272);
    			attr_dev(div5, "class", "flex flex-col h-screen bg-gray-50");
    			add_location(div5, file, 752, 0, 24193);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div5, anchor);
    			mount_component(modal, div5, null);
    			append_dev(div5, t0);
    			append_dev(div5, header);
    			append_dev(header, div2);
    			append_dev(div2, div0);
    			append_dev(div0, select);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(select, null);
    			}

    			select_option(select, /*selectedWs*/ ctx[11]);
    			append_dev(div2, t1);
    			append_dev(div2, div1);
    			mount_component(cloudicon, div1, null);
    			append_dev(div5, t2);
    			append_dev(div5, nav);
    			append_dev(nav, input);
    			input.checked = /*opened*/ ctx[0];
    			append_dev(nav, t3);
    			append_dev(nav, label);
    			append_dev(label, span);
    			append_dev(nav, t4);
    			append_dev(nav, ul0);
    			append_dev(ul0, li0);
    			append_dev(li0, a0);
    			append_dev(ul0, t6);
    			append_dev(ul0, li1);
    			append_dev(li1, a1);
    			append_dev(ul0, t8);
    			append_dev(ul0, li2);
    			append_dev(li2, a2);
    			append_dev(ul0, t10);
    			append_dev(ul0, li3);
    			append_dev(li3, a3);
    			append_dev(ul0, t12);
    			append_dev(ul0, li4);
    			append_dev(li4, a4);
    			append_dev(ul0, t14);
    			append_dev(ul0, li5);
    			append_dev(li5, a5);
    			append_dev(ul0, t16);
    			append_dev(ul0, li6);
    			append_dev(li6, a6);
    			append_dev(div5, t18);
    			append_dev(div5, main);
    			append_dev(main, ul1);
    			append_dev(ul1, div3);
    			if_blocks[current_block_type_index].m(div3, null);
    			append_dev(div3, t19);
    			mount_component(route, div3, null);
    			append_dev(div5, t20);
    			append_dev(div5, footer);
    			append_dev(footer, div4);
    			current = true;

    			if (!mounted) {
    				dispose = [
    					listen_dev(select, "change", /*select_change_handler*/ ctx[26]),
    					listen_dev(select, "change", /*change_handler*/ ctx[27], false, false, false),
    					listen_dev(input, "change", /*input_change_handler*/ ctx[28]),
    					listen_dev(input, "change", /*change_handler_1*/ ctx[29], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, dirty) {
    			const modal_changes = {};
    			if (dirty[0] & /*showModalFlag*/ 4) modal_changes.show = /*showModalFlag*/ ctx[2];
    			modal.$set(modal_changes);

    			if (dirty[0] & /*deviceList*/ 16384) {
    				each_value = /*deviceList*/ ctx[14];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    					} else {
    						each_blocks[i] = create_each_block(child_ctx);
    						each_blocks[i].c();
    						each_blocks[i].m(select, null);
    					}
    				}

    				for (; i < each_blocks.length; i += 1) {
    					each_blocks[i].d(1);
    				}

    				each_blocks.length = each_value.length;
    			}

    			if (dirty[0] & /*selectedWs, deviceList*/ 18432) {
    				select_option(select, /*selectedWs*/ ctx[11]);
    			}

    			const cloudicon_changes = {};

    			if (dirty[0] & /*socketConnected*/ 1024) cloudicon_changes.color = /*socketConnected*/ ctx[10] === true
    			? "text-green-500"
    			: "text-red-500";

    			cloudicon.$set(cloudicon_changes);

    			if (dirty[0] & /*opened*/ 1) {
    				input.checked = /*opened*/ ctx[0];
    			}

    			let previous_block_index = current_block_type_index;
    			current_block_type_index = select_block_type(ctx);

    			if (current_block_type_index === previous_block_index) {
    				if_blocks[current_block_type_index].p(ctx, dirty);
    			} else {
    				group_outros();

    				transition_out(if_blocks[previous_block_index], 1, 1, () => {
    					if_blocks[previous_block_index] = null;
    				});

    				check_outros();
    				if_block = if_blocks[current_block_type_index];

    				if (!if_block) {
    					if_block = if_blocks[current_block_type_index] = if_block_creators[current_block_type_index](ctx);
    					if_block.c();
    				} else {
    					if_block.p(ctx, dirty);
    				}

    				transition_in(if_block, 1);
    				if_block.m(div3, t19);
    			}

    			const route_changes = {};

    			if (dirty[0] & /*deviceList, newDevice*/ 20480 | dirty[3] & /*$$scope*/ 1) {
    				route_changes.$$scope = { dirty, ctx };
    			}

    			route.$set(route_changes);

    			if (!current || dirty[0] & /*opened, preventMove*/ 3 && main_class_value !== (main_class_value = "flex-1 overflow-y-auto p-0 " + (/*opened*/ ctx[0] === true && !/*preventMove*/ ctx[1]
    			? 'ml-36'
    			: 'ml-0'))) {
    				attr_dev(main, "class", main_class_value);
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(modal.$$.fragment, local);
    			transition_in(cloudicon.$$.fragment, local);
    			transition_in(if_block);
    			transition_in(route.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(modal.$$.fragment, local);
    			transition_out(cloudicon.$$.fragment, local);
    			transition_out(if_block);
    			transition_out(route.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div5);
    			destroy_component(modal);
    			destroy_each(each_blocks, detaching);
    			destroy_component(cloudicon);
    			if_blocks[current_block_type_index].d();
    			destroy_component(route);
    			mounted = false;
    			run_all(dispose);
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

    function combineArrays(A, B) {
    	var ids = new Set(A.map(d => d.ip));
    	let output = [...A, ...B.filter(d => !ids.has(d.ip))];
    	return output;
    }

    //****************************************************************json******************************************************************/
    function getJsonObject(array, number) {
    	let num = 0;
    	let out = {};

    	array.forEach(object => {
    		if (num === number) {
    			out = object;
    		}

    		num++;
    	});

    	return out;
    }

    async function handleSubmit(url) {
    	try {
    		console.log(url);
    		let res = await fetch(url, { mode: "no-cors", method: "GET" });

    		if (res.ok) {
    			console.log("OK", res.status);
    		} else {
    			console.log("error", res.status); //console.log(url);
    		} //console.log(url);
    	} catch(e) {
    		console.log(e);
    	}
    }

    async function getRequestJson(url) {
    	let res = await fetch(url, { mode: "no-cors", method: "GET" });

    	if (res.ok) {
    		configSetupJson = await res.json();
    	} else {
    		console.log("error", res.status);
    	}
    }

    function instance($$self, $$props, $$invalidate) {
    	let $router;
    	validate_store(f, 'router');
    	component_subscribe($$self, f, $$value => $$invalidate(57, $router = $$value));
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('App', slots, []);
    	f.mode.hash();

    	//****************************************************constants section*********************************************************/
    	//******************************************************************************************************************************/
    	let debug = true;

    	let LOG_MAX_MESSAGES = 10;
    	let reconnectTimeout = 60000;
    	let opened = false;
    	let preventMove = false;

    	//****************************************************variable section**********************************************************/
    	//******************************************************************************************************************************/
    	let myip = document.location.hostname;

    	//let myip = "192.168.88.235";
    	//Flags
    	let showInput = false;

    	let showModalFlag = false;
    	let additionalParams = false;

    	//dashboard
    	let pages = [];

    	//configuration
    	let configJson = [];

    	let configJsonFlag = false;
    	let configJsonParced = false;
    	let widgetsJson = [];
    	let widgetsJsonFlag = false;
    	let widgetsJsonParced = false;
    	let itemsJson = [];
    	let itemsJsonFlag = false;
    	let itemsJsonParced = false;
    	let layoutJson = [];
    	let layoutJsonFlag = false;
    	let layoutJsonParced = false;
    	let settingsJson = {};
    	let settingsJsonFlag = false;
    	let settingsJsonParced = false;
    	let ssidJson = {};
    	let ssidJsonParced = false;

    	//web sockets
    	let socket = [];

    	let socketConnected = false;
    	let selectedDeviceData = undefined;
    	let selectedWs = 0;
    	let flag = true;
    	let newDevice = {};
    	let coreMessages = [];
    	let oneOfJsonPackageError = false;
    	let deviceList = [];

    	deviceList = [
    		{
    			name: "--",
    			id: "--",
    			ip: myip,
    			status: false
    		}
    	];

    	let incDeviceList = [];
    	let incDeviceListParced = false;

    	//***********************************************************blob**************************************************************/
    	var MyBlobBuilder = function () {
    		this.parts = [];
    	};

    	MyBlobBuilder.prototype.append = function (part) {
    		this.parts.push(part);
    		this.blob = undefined; // Invalidate the blob
    	};

    	MyBlobBuilder.prototype.getBlob = function () {
    		if (!this.blob) {
    			this.blob = new Blob(this.parts, { type: "binary" });
    		}

    		return this.blob;
    	};

    	MyBlobBuilder.prototype.clear = function () {
    		this.parts = [];
    	};

    	//***********************************************************navigation********************************************************/
    	let currentPageName = undefined;

    	var configJsonBlob = new MyBlobBuilder();
    	var widgetsJsonBlob = new MyBlobBuilder();
    	var itemsJsonBlob = new MyBlobBuilder();
    	var layoutJsonBlob = new MyBlobBuilder();
    	var settingsJsonBlob = new MyBlobBuilder();
    	f.subscribe(handleNavigation);

    	function handleNavigation() {
    		clearData();
    		currentPageName = $router.path.toString();
    		console.log("[i]", "user on page:", currentPageName);
    		sendCurrentPageName();
    	}

    	function sendCurrentPageName() {
    		if (selectedWs !== undefined) {
    			wsSendMsg(selectedWs, currentPageName);
    		}
    	}

    	//****************************************************web sockets section******************************************************/
    	function connectToAllDevices() {
    		//closeAllConnection();
    		//socket = [];
    		getSelectedDeviceData(selectedWs);

    		let ws = 0;

    		deviceList.forEach(device => {
    			//if (debug) console.log("[i]", device.name, ws, device.ip, device.id);
    			device.ws = ws;

    			if (!device.status) {
    				wsConnect(ws);
    				wsEventAdd(ws);
    			}

    			ws++;
    		});

    		$$invalidate(14, deviceList);
    	} //socketConnected = selectedDeviceData.status;

    	function closeAllConnection() {
    		let s;

    		for (s in socket) {
    			socket[s].close();
    		}
    	}

    	function markDeviceStatus(ws, status) {
    		deviceList.forEach(device => {
    			if (device.ws === ws) {
    				device.status = status;

    				if (debug) {
    					if (device.status) {
    						console.log("[i]", device.ip, "status online");
    					} else {
    						console.log("[i]", device.ip, "status offline");
    					}
    				}
    			}
    		});

    		$$invalidate(14, deviceList);
    		getSelectedDeviceData(selectedWs);
    		$$invalidate(10, socketConnected = selectedDeviceData.status);
    	}

    	function getDeviceStatus(ws) {
    		let ret = false;

    		deviceList.forEach(device => {
    			if (ws === device.ws) {
    				ret = device.status;
    			}
    		});

    		return ret;
    	}

    	function wsConnect(ws) {
    		let ip = getIP(ws);

    		if (ip === "error") {
    			if (debug) console.log("[e]", "device list wrong");
    		} else {
    			socket[ws] = new WebSocket("ws://" + ip + ":81");
    			socket.binaryType = "blob";

    			//socket[ws] = new WebSocket("ws://" + ip + "/ws");
    			if (debug) console.log("[i]", ip, "started connecting...");
    		}
    	}

    	function getIP(ws) {
    		let ret = "error";

    		deviceList.forEach(device => {
    			if (ws === device.ws) {
    				ret = device.ip;
    			}
    		});

    		return ret;
    	}

    	function wsEventAdd(ws) {
    		if (socket[ws]) {
    			let ip = getIP(ws);
    			if (debug) console.log("[i]", ip, "web socket events added");

    			socket[ws].addEventListener("open", function (event) {
    				if (debug) console.log("[i]", ip, "completed connecting");
    				markDeviceStatus(ws, true);
    				sendCurrentPageName();
    			}); //socket[ws].send("HELLO");

    			socket[ws].addEventListener("message", function (event) {
    				if (typeof event.data === "string") {
    					let data = event.data;

    					//if (debug) console.log("[i]", getIP(ws), "msg received", data); //
    					//сборщик statusJson сообщений======================================
    					if (data.includes("status")) {
    						if (IsJsonParse(data)) {
    							let statusJson = JSON.parse(data);
    							udatelayoutJson(statusJson);
    							wigetsUpdate();
    							if (debug) console.log("[i]", "status json parced: ", statusJson);
    						}
    					}

    					//сборщик ssidJson сообщений======================================
    					if (data.includes("ssid")) {
    						if (IsJsonParse(data)) {
    							$$invalidate(9, ssidJson = JSON.parse(data));
    							delete ssidJson.ssid;
    							$$invalidate(9, ssidJson);
    							ssidJsonParced = true;
    							if (debug) console.log("[i]", "ssid json parced");
    						}
    					}

    					//сборщик deviceList сообщений======================================
    					if (data.includes("devicelist")) {
    						if (IsJsonParse(data)) {
    							incDeviceList = JSON.parse(data);
    							delete incDeviceList.devicelist;
    							incDeviceList = incDeviceList;
    							incDeviceListParced = true;
    							$$invalidate(14, deviceList = combineArrays(deviceList, incDeviceList));
    							$$invalidate(14, deviceList);
    							whenDeviceListWasUpdated();
    							connectToAllDevices();
    							if (debug) console.log("[i]", "incDeviceList json parced", incDeviceList);
    						}
    					}

    					//сборщик configJson пакетов========================================
    					if (data === "/st/config.json") {
    						//if (debug) console.log("[i]", "configJson start!");
    						configJsonFlag = true;
    					}

    					if (data === "/end/config.json") {
    						//if (debug) console.log("[i]", "configJson end!");
    						configJsonFlag = false;

    						var bb = configJsonBlob.getBlob();
    						let configJsonReader = new FileReader();
    						configJsonReader.readAsText(bb);

    						configJsonReader.onload = () => {
    							let configJsonResult = configJsonReader.result;

    							if (IsJsonParse(configJsonResult)) {
    								$$invalidate(4, configJson = JSON.parse(configJsonResult));
    								$$invalidate(4, configJson);
    								configJsonParced = true;
    								if (debug) console.log("[ii]", "configJson parced!");
    							}
    						};
    					}

    					//сборщик widgetsJson пакетов========================================
    					if (data === "/st/widgets.json") {
    						//if (debug) console.log("[i]", "widgetsJson start!");
    						widgetsJsonFlag = true;
    					}

    					if (data === "/end/widgets.json") {
    						//if (debug) console.log("[i]", "widgetsJson end!");
    						widgetsJsonFlag = false;

    						var bb = widgetsJsonBlob.getBlob();
    						let widgetsJsonReader = new FileReader();
    						widgetsJsonReader.readAsText(bb);

    						widgetsJsonReader.onload = () => {
    							let widgetsJsonResult = widgetsJsonReader.result;

    							if (IsJsonParse(widgetsJsonResult)) {
    								$$invalidate(5, widgetsJson = JSON.parse(widgetsJsonResult));
    								$$invalidate(5, widgetsJson);
    								widgetsJsonParced = true;
    								if (debug) console.log("[ii]", "widgetsJson parced!");
    							}
    						};
    					}

    					//сборщик itemsJson пакетов========================================
    					if (data === "/st/items.json") {
    						//if (debug) console.log("[i]", "itemsJson start!");
    						itemsJsonFlag = true;
    					}

    					if (data === "/end/items.json") {
    						//if (debug) console.log("[i]", "itemsJson end!");
    						itemsJsonFlag = false;

    						var bb = itemsJsonBlob.getBlob();
    						let itemsJsonReader = new FileReader();
    						itemsJsonReader.readAsText(bb);

    						itemsJsonReader.onload = () => {
    							let itemsJsonResult = itemsJsonReader.result;

    							if (IsJsonParse(itemsJsonResult)) {
    								$$invalidate(6, itemsJson = JSON.parse(itemsJsonResult));
    								$$invalidate(6, itemsJson);
    								itemsJsonParced = true;
    								if (debug) console.log("[ii]", "itemsJson parced!");
    							}
    						};
    					}

    					//сборщик layoutJson пакетов========================================
    					if (data === "/st/layout.json") {
    						//if (debug) console.log("[i]", "layoutJson start!");
    						layoutJsonFlag = true;
    					}

    					if (data === "/end/layout.json") {
    						//if (debug) console.log("[i]", "layoutJson end!");
    						layoutJsonFlag = false;

    						var bb = layoutJsonBlob.getBlob();
    						let layoutJsonReader = new FileReader();
    						layoutJsonReader.readAsText(bb);

    						layoutJsonReader.onload = () => {
    							let layoutJsonResult = layoutJsonReader.result;

    							if (IsJsonParse(layoutJsonResult)) {
    								$$invalidate(7, layoutJson = JSON.parse(layoutJsonResult));
    								$$invalidate(7, layoutJson);
    								wigetsUpdate();
    								layoutJsonParced = true;
    								if (debug) console.log("[ii]", "layoutJson parced!");
    							}
    						};
    					}

    					//сборщик settingsJson пакетов========================================
    					if (data === "/st/settings.json") {
    						//if (debug) console.log("[i]", "settingsJson start!");
    						settingsJsonFlag = true;
    					}

    					if (data === "/end/settings.json") {
    						//if (debug) console.log("[i]", "settingsJson end!");
    						settingsJsonFlag = false;

    						var bb = settingsJsonBlob.getBlob();
    						let settingsJsonReader = new FileReader();
    						settingsJsonReader.readAsText(bb);

    						settingsJsonReader.onload = () => {
    							let settingsJsonResult = settingsJsonReader.result;

    							if (IsJsonParse(settingsJsonResult)) {
    								$$invalidate(8, settingsJson = JSON.parse(settingsJsonResult));
    								$$invalidate(8, settingsJson);
    								wigetsUpdate();
    								settingsJsonParced = true;
    								updateThisDeviceInList();
    								if (debug) console.log("[ii]", "settingsJson parced!");
    							}
    						};
    					}
    				}

    				if (event.data instanceof Blob) {
    					if (configJsonFlag) configJsonBlob.append(event.data);
    					if (widgetsJsonFlag) widgetsJsonBlob.append(event.data);
    					if (itemsJsonFlag) itemsJsonBlob.append(event.data);
    					if (layoutJsonFlag) layoutJsonBlob.append(event.data);
    					if (settingsJsonFlag) settingsJsonBlob.append(event.data);
    				}
    			});

    			socket[ws].addEventListener("close", event => {
    				if (debug) console.log("[e]", ip, "connection closed");
    				markDeviceStatus(ws, false);
    			});

    			socket[ws].addEventListener("error", function (event) {
    				if (debug) console.log("[e]", ip, "connection error");
    				markDeviceStatus(ws, false);
    			});
    		} else {
    			if (debug) console.log("[e]", "socket not exist");
    		}
    	}

    	function saveConfig() {
    		wsSendMsg(selectedWs, "/tuoyal" + JSON.stringify(generateLayout()));
    		wsSendMsg(selectedWs, "/gifnoc" + JSON.stringify(configJson));
    		clearData();
    		sendCurrentPageName();
    	}

    	function saveSettings() {
    		wsSendMsg(selectedWs, "/cennoc" + JSON.stringify(settingsJson));
    		clearData();
    		sendCurrentPageName();
    	}

    	function generateLayout() {
    		let layout = [];

    		for (let i = 0; i < configJson.length; i++) {
    			let config = Object.assign({}, configJson[i]);
    			let setWidget = config.widget;
    			let error = true;

    			for (let w = 0; w < widgetsJson.length; w++) {
    				if (setWidget === widgetsJson[w].name) {
    					let widget = Object.assign({}, widgetsJson[w]);
    					widget.page = config.page;
    					widget.descr = config.descr;

    					//widget.id = config.id;
    					//widget.ws = selectedWs;
    					widget.topic = settingsJson.root + "/" + config.id;

    					layout.push(widget);
    					error = false;
    					break;
    				} else {
    					error = true;
    				}
    			}

    			if (error) console.log("[e]", "error, widget not found: " + setWidget);
    		}

    		if (debug) console.log("[i] layout:", JSON.stringify(layout));
    		return layout;
    	}

    	function udatelayoutJson(newStatusJson) {
    		for (let i = 0; i < layoutJson.length; i++) {
    			let topic = layoutJson[i].topic;

    			if (topic === newStatusJson.topic) {
    				$$invalidate(7, layoutJson[i].status = newStatusJson.status, layoutJson);
    				break;
    			}
    		}
    	}

    	function clearData() {
    		$$invalidate(4, configJson = []);
    		configJsonBlob.clear();
    		$$invalidate(5, widgetsJson = []);
    		widgetsJsonBlob.clear();
    		$$invalidate(6, itemsJson = []);
    		itemsJsonBlob.clear();
    		$$invalidate(7, layoutJson = []);
    		layoutJsonBlob.clear();
    		$$invalidate(8, settingsJson = {});
    		settingsJsonBlob.clear();
    		configJsonParced = false;
    		widgetsJsonParced = false;
    		itemsJsonParced = false;
    		layoutJsonParced = false;
    		settingsJsonParced = false;
    		ssidJsonParced = false;
    		if (debug) console.log("[i]", "all app data cleared");
    	}

    	function wsPush(ws, topic, status) {
    		let msg = topic + " " + status;
    		if (debug) console.log("[i]", "send to ws msg:", msg);
    		wsSendMsg(ws, msg);
    	}

    	function wsTestMsgTask() {
    		setTimeout(wsTestMsgTask, reconnectTimeout);
    		if (debug) console.log("[i]", "----timer tick----");

    		if (!flag) {
    			deviceList.forEach(device => {
    				if (!getDeviceStatus(device.ws)) {
    					wsConnect(device.ws);
    					wsEventAdd(device.ws);
    				} else {
    					wsSendMsg(device.ws, "tst");
    				}
    			});
    		}

    		flag = false;
    	}

    	function wsSendMsg(ws, msg) {
    		if (socket[ws] && socket[ws].readyState === 1) {
    			socket[ws].send(msg);
    			if (debug) console.log("[i]", getIP(ws), "msg send success", msg);
    		} else {
    			if (debug) console.log("[e]", getIP(ws), "msg not send", msg);
    		}
    	}

    	//***********************************************************dashboard***************************************************************/
    	function findNewPage() {
    		$$invalidate(3, pages = []);
    		const newPage = Array.from(new Set(Array.from(layoutJson, ({ page }) => page)));

    		newPage.forEach(function (item, i, arr) {
    			$$invalidate(3, pages = [...pages, JSON.parse(JSON.stringify({ page: item }))]);
    		});

    		pages.sort(function (a, b) {
    			if (a.page < b.page) {
    				return -1;
    			}

    			if (a.page > b.page) {
    				return 1;
    			}

    			return 0;
    		});
    	}

    	function wigetsUpdate() {
    		findNewPage();
    	}

    	//***********************************************************logging******************************************************************/
    	const addCoreMsg = msg => {
    		if (coreMessages.length > Number(LOG_MAX_MESSAGES)) {
    			$$invalidate(13, coreMessages = coreMessages.slice(0));
    		}

    		const time = new Date().getTime();
    		$$invalidate(13, coreMessages = [...coreMessages, { time, msg }]);

    		coreMessages.sort(function (a, b) {
    			if (a.time > b.time) {
    				return -1;
    			}

    			if (a.time < b.time) {
    				return 1;
    			}

    			return 0;
    		});
    	};

    	function whenDeviceListWasUpdated() {
    		getSelectedDeviceData(selectedWs);
    		$$invalidate(10, socketConnected = selectedDeviceData.status);
    	}

    	function devicesDropdownChange() {
    		whenDeviceListWasUpdated();
    		clearData();
    		sendCurrentPageName();
    		if (debug) console.log("[i]", "user selected device:", selectedDeviceData.name);

    		if (selectedDeviceData.ip === myip) {
    			if (debug) console.log("[i]", "user selected original device", selectedDeviceData.name);
    		}
    	}

    	function getSelectedDeviceData(ws) {
    		for (let i = 0; i < deviceList.length; i++) {
    			let device = deviceList[i];

    			if (device.ws === ws) {
    				selectedDeviceData = device;
    				break;
    			}
    		}
    	}

    	function devListSave() {
    		//createDefaultDevice();
    		if (!showInput) {
    			if (newDevice.name !== undefined && newDevice.ip !== undefined && newDevice.id !== undefined) {
    				$$invalidate(12, newDevice.status = false, newDevice);
    				deviceList.push(newDevice);
    				$$invalidate(14, deviceList);
    				$$invalidate(12, newDevice = {});
    				whenDeviceListWasUpdated();
    				connectToAllDevices();
    				if (debug) console.log("[i]", "selected device:", selectedDeviceData);
    			} else {
    				if (debug) console.log("[e]", "wrong data");
    			}
    		}
    	}

    	function updateThisDeviceInList() {
    		for (let i = 0; i < deviceList.length; i++) {
    			let device = deviceList[i];

    			if (device.ip === myip) {
    				device.name = settingsJson.name;
    				device.id = settingsJson.id;
    				$$invalidate(8, settingsJson);
    				break;
    			}
    		}
    	}

    	const syntaxHighlight = json => {
    		try {
    			json = JSON.stringify(JSON.parse(json), null, 4);
    		} catch(e) {
    			return json;
    		}

    		json = json.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");

    		json = json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
    			return match;
    		});

    		return json;
    	};

    	function IsJsonParse(str) {
    		try {
    			JSON.parse(str);
    		} catch(e) {
    			oneOfJsonPackageError = true;
    			if (debug) console.log("[e]", "json error");
    			return false;
    		}

    		oneOfJsonPackageError = false;
    		return true;
    	}

    	//пример как формировать массив json
    	function createWidgetsDropdown() {
    		let widgetsDropdown = [];

    		widgetsJson.forEach(widget => {
    			widgetsDropdown.push({ id: widget.name, val: widget.rus });
    		});

    		widgetsDropdown = widgetsDropdown;
    		if (debug) console.log("[i]", widgetsDropdown);
    	}

    	//**********************************************************post and get*****************************************************************/
    	//editRequest("192.168.88.235", "data data data data", "file.json")
    	function editRequest(url, data, filename) {
    		if (debug) console.log("[i]", "request for edit file");
    		var xmlHttp = new XMLHttpRequest();
    		var formData = new FormData();
    		formData.append("data", new Blob([data], { type: "text/json" }), "/" + filename);
    		xmlHttp.open("POST", "http://" + url + "/edit");

    		xmlHttp.onload = function () {
    			
    		}; //во время загрузки

    		xmlHttp.send(formData);
    	}

    	function showAdditionalParams(id) {
    		additionalParams = true;
    		if (debug) console.log("[i]", "user open add params ", id);
    	}

    	//**********************************************************modal*************************************************************************/
    	function showModal() {
    		$$invalidate(2, showModalFlag = !showModalFlag);
    	}

    	function onCheck() {
    		let width = screen.width;
    		console.log("width", width);

    		if (width < 900) {
    			$$invalidate(1, preventMove = true);
    		} else {
    			$$invalidate(1, preventMove = false);
    		}
    	}

    	//************************************************elements and presets dropdown************************************************************/
    	function ssidDropdownClick() {
    		wsSendMsg(selectedWs, "/scan");
    	}

    	//*******************************************************initialisation********************************************************************/
    	onMount(async () => {
    		console.log("[i]", "mounted");
    		connectToAllDevices();
    		wsTestMsgTask();
    		whenDeviceListWasUpdated();
    		findNewPage();
    	});

    	const writable_props = [];

    	Object_1.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console_1.warn(`<App> was created with unknown prop '${key}'`);
    	});

    	function select_change_handler() {
    		selectedWs = select_value(this);
    		$$invalidate(11, selectedWs);
    		$$invalidate(14, deviceList);
    	}

    	const change_handler = () => devicesDropdownChange();

    	function input_change_handler() {
    		opened = this.checked;
    		$$invalidate(0, opened);
    	}

    	const change_handler_1 = () => onCheck();
    	const func = (ws, topic, status) => wsPush(ws, topic, status);
    	const func_1 = () => saveConfig();
    	const func_2 = () => ssidDropdownClick();
    	const func_3 = () => saveSettings();
    	const func_4 = () => showModal();
    	const func_5 = json => syntaxHighlight(json);
    	const func_6 = () => devListSave();

    	$$self.$capture_state = () => ({
    		onMount,
    		Route,
    		router: f,
    		active: A,
    		Alarm,
    		Modal,
    		DashboardPage: Dashboard,
    		ConfigPage: Config,
    		ConnectionPage: Connection,
    		UtilitiesPage: Utilities,
    		LogPage: Log,
    		ListPage: List,
    		AboutPage: About,
    		CloudIcon: Cloud,
    		debug,
    		LOG_MAX_MESSAGES,
    		reconnectTimeout,
    		opened,
    		preventMove,
    		myip,
    		showInput,
    		showModalFlag,
    		additionalParams,
    		pages,
    		configJson,
    		configJsonFlag,
    		configJsonParced,
    		widgetsJson,
    		widgetsJsonFlag,
    		widgetsJsonParced,
    		itemsJson,
    		itemsJsonFlag,
    		itemsJsonParced,
    		layoutJson,
    		layoutJsonFlag,
    		layoutJsonParced,
    		settingsJson,
    		settingsJsonFlag,
    		settingsJsonParced,
    		ssidJson,
    		ssidJsonParced,
    		socket,
    		socketConnected,
    		selectedDeviceData,
    		selectedWs,
    		flag,
    		newDevice,
    		coreMessages,
    		oneOfJsonPackageError,
    		deviceList,
    		incDeviceList,
    		incDeviceListParced,
    		MyBlobBuilder,
    		currentPageName,
    		configJsonBlob,
    		widgetsJsonBlob,
    		itemsJsonBlob,
    		layoutJsonBlob,
    		settingsJsonBlob,
    		handleNavigation,
    		sendCurrentPageName,
    		connectToAllDevices,
    		closeAllConnection,
    		markDeviceStatus,
    		getDeviceStatus,
    		wsConnect,
    		getIP,
    		wsEventAdd,
    		saveConfig,
    		saveSettings,
    		generateLayout,
    		udatelayoutJson,
    		clearData,
    		wsPush,
    		wsTestMsgTask,
    		wsSendMsg,
    		findNewPage,
    		wigetsUpdate,
    		addCoreMsg,
    		combineArrays,
    		whenDeviceListWasUpdated,
    		devicesDropdownChange,
    		getSelectedDeviceData,
    		devListSave,
    		updateThisDeviceInList,
    		getJsonObject,
    		syntaxHighlight,
    		IsJsonParse,
    		createWidgetsDropdown,
    		editRequest,
    		handleSubmit,
    		getRequestJson,
    		showAdditionalParams,
    		showModal,
    		onCheck,
    		ssidDropdownClick,
    		$router
    	});

    	$$self.$inject_state = $$props => {
    		if ('debug' in $$props) debug = $$props.debug;
    		if ('LOG_MAX_MESSAGES' in $$props) LOG_MAX_MESSAGES = $$props.LOG_MAX_MESSAGES;
    		if ('reconnectTimeout' in $$props) reconnectTimeout = $$props.reconnectTimeout;
    		if ('opened' in $$props) $$invalidate(0, opened = $$props.opened);
    		if ('preventMove' in $$props) $$invalidate(1, preventMove = $$props.preventMove);
    		if ('myip' in $$props) myip = $$props.myip;
    		if ('showInput' in $$props) $$invalidate(15, showInput = $$props.showInput);
    		if ('showModalFlag' in $$props) $$invalidate(2, showModalFlag = $$props.showModalFlag);
    		if ('additionalParams' in $$props) additionalParams = $$props.additionalParams;
    		if ('pages' in $$props) $$invalidate(3, pages = $$props.pages);
    		if ('configJson' in $$props) $$invalidate(4, configJson = $$props.configJson);
    		if ('configJsonFlag' in $$props) configJsonFlag = $$props.configJsonFlag;
    		if ('configJsonParced' in $$props) configJsonParced = $$props.configJsonParced;
    		if ('widgetsJson' in $$props) $$invalidate(5, widgetsJson = $$props.widgetsJson);
    		if ('widgetsJsonFlag' in $$props) widgetsJsonFlag = $$props.widgetsJsonFlag;
    		if ('widgetsJsonParced' in $$props) widgetsJsonParced = $$props.widgetsJsonParced;
    		if ('itemsJson' in $$props) $$invalidate(6, itemsJson = $$props.itemsJson);
    		if ('itemsJsonFlag' in $$props) itemsJsonFlag = $$props.itemsJsonFlag;
    		if ('itemsJsonParced' in $$props) itemsJsonParced = $$props.itemsJsonParced;
    		if ('layoutJson' in $$props) $$invalidate(7, layoutJson = $$props.layoutJson);
    		if ('layoutJsonFlag' in $$props) layoutJsonFlag = $$props.layoutJsonFlag;
    		if ('layoutJsonParced' in $$props) layoutJsonParced = $$props.layoutJsonParced;
    		if ('settingsJson' in $$props) $$invalidate(8, settingsJson = $$props.settingsJson);
    		if ('settingsJsonFlag' in $$props) settingsJsonFlag = $$props.settingsJsonFlag;
    		if ('settingsJsonParced' in $$props) settingsJsonParced = $$props.settingsJsonParced;
    		if ('ssidJson' in $$props) $$invalidate(9, ssidJson = $$props.ssidJson);
    		if ('ssidJsonParced' in $$props) ssidJsonParced = $$props.ssidJsonParced;
    		if ('socket' in $$props) socket = $$props.socket;
    		if ('socketConnected' in $$props) $$invalidate(10, socketConnected = $$props.socketConnected);
    		if ('selectedDeviceData' in $$props) selectedDeviceData = $$props.selectedDeviceData;
    		if ('selectedWs' in $$props) $$invalidate(11, selectedWs = $$props.selectedWs);
    		if ('flag' in $$props) flag = $$props.flag;
    		if ('newDevice' in $$props) $$invalidate(12, newDevice = $$props.newDevice);
    		if ('coreMessages' in $$props) $$invalidate(13, coreMessages = $$props.coreMessages);
    		if ('oneOfJsonPackageError' in $$props) oneOfJsonPackageError = $$props.oneOfJsonPackageError;
    		if ('deviceList' in $$props) $$invalidate(14, deviceList = $$props.deviceList);
    		if ('incDeviceList' in $$props) incDeviceList = $$props.incDeviceList;
    		if ('incDeviceListParced' in $$props) incDeviceListParced = $$props.incDeviceListParced;
    		if ('MyBlobBuilder' in $$props) MyBlobBuilder = $$props.MyBlobBuilder;
    		if ('currentPageName' in $$props) currentPageName = $$props.currentPageName;
    		if ('configJsonBlob' in $$props) configJsonBlob = $$props.configJsonBlob;
    		if ('widgetsJsonBlob' in $$props) widgetsJsonBlob = $$props.widgetsJsonBlob;
    		if ('itemsJsonBlob' in $$props) itemsJsonBlob = $$props.itemsJsonBlob;
    		if ('layoutJsonBlob' in $$props) layoutJsonBlob = $$props.layoutJsonBlob;
    		if ('settingsJsonBlob' in $$props) settingsJsonBlob = $$props.settingsJsonBlob;
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		opened,
    		preventMove,
    		showModalFlag,
    		pages,
    		configJson,
    		widgetsJson,
    		itemsJson,
    		layoutJson,
    		settingsJson,
    		ssidJson,
    		socketConnected,
    		selectedWs,
    		newDevice,
    		coreMessages,
    		deviceList,
    		showInput,
    		saveConfig,
    		saveSettings,
    		wsPush,
    		wigetsUpdate,
    		devicesDropdownChange,
    		devListSave,
    		syntaxHighlight,
    		showModal,
    		onCheck,
    		ssidDropdownClick,
    		select_change_handler,
    		change_handler,
    		input_change_handler,
    		change_handler_1,
    		func,
    		func_1,
    		func_2,
    		func_3,
    		func_4,
    		func_5,
    		func_6
    	];
    }

    class App extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance, create_fragment, safe_not_equal, {}, null, [-1, -1, -1, -1]);

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "App",
    			options,
    			id: create_fragment.name
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

})();
//# sourceMappingURL=bundle.js.map
