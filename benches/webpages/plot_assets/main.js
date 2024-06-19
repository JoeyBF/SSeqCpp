/* created by Weinan Lin */

/************************************************************************
 *                          config variables
 ***********************************************************************/
const urlParams = new URLSearchParams(window.location.search);
const DATA_NAME = urlParams.get("data") || "S0";
const COUNT_MAPS = DATA_NAME.split("__").length - 1;
const MODE = ["ss", "map", "cofseq"][COUNT_MAPS];
var DATA_JSON = {};

const CONFIG = {
    x_max: MODE === "cofseq" ? 270 * 3 : 270,
    x_min_cofseq: [0, 0, 0],
    y_max: 130,
    y_max_grid: 256,
    x_max_init: 80,
    margin: 30,
    axis_text_sep_screen: 60,
    camera_zoom_rate: navigator.userAgent.match("Macintosh") ? 1.1 : 1.2,
    camera_translate_pixels: 100,
    R_PERM: 1000,
    plot_batchSize: 1000,
};

const CONFIG_DYNAMIC = {
    status: "start",
    camera_unit_screen_init: (window.innerWidth - CONFIG.margin) / (CONFIG.x_max_init + 1),
    camera_unit_screen_min:
        (window.innerWidth - CONFIG.margin) / (CONFIG.x_max + 1),
    camera_unit_screen_max: Math.min(window.innerWidth, window.innerHeight) - 30,
    page: 2,
    dashedLevel: 5,
    showLines: "All diff",
};


/************************************************************************
 *                          elements
 ***********************************************************************/

const svg_ss = document.getElementById("svg_ss");
svg_ss.setAttribute("width", window.innerWidth);
svg_ss.setAttribute("height", window.innerHeight);
const g_svg = document.getElementById("g_svg");
g_svg.setAttribute(
    "transform",
    "translate(0," + window.innerHeight + ") scale(1,-1)"
);
const g_plot = document.getElementById("g_plot");
const g_bullets = {
    "black": document.getElementById("g_bullets_black"),
    "blue": document.getElementById("g_bullets_blue"),
    "grey": document.getElementById("g_bullets_grey"),
};
const g_strtlines = document.getElementById("g_strtlines");
const g_strtlines_purple = document.getElementById("g_strtlines_purple");
const g_labels = document.getElementById("g_labels");
const g_xaxis = document.getElementById("g_xaxis");
const g_yaxis = document.getElementById("g_yaxis");
const g_diff_lines = {
    0: document.getElementById("g_difflines_black"), ////////
    1: document.getElementById("g_difflines_blue"), ////////
    2: document.getElementById("g_difflines_deepskyblue"),
    3: document.getElementById("g_difflines_red"),
    4: document.getElementById("g_difflines_green"),
    5: document.getElementById("g_difflines_blue"),
    6: document.getElementById("g_difflines_orange"),
};

const div_binfo_style = document.getElementById("div_binfo").style;
const circle_mouseon = document.getElementById("circle_mouseon");
const rect_selected = document.getElementById("rect_selected");
const p_deg = document.getElementById("p_deg");
const p_base = document.getElementById("p_base");
const p_latex = document.getElementById("p_latex");
const p_diff = document.getElementById("p_diff");
const g_prod = document.getElementById("g_prod");
const g_map = document.getElementById("g_map");
const div_menu_style = document.getElementById("div_menu").style;

const rect_separator = document.getElementById("rect_separator");
const rect_second_ss = document.getElementById("rect_second_ss");
const select_page = document.getElementById("select_page");

/* Resize window */
function windowResize() {
    svg_ss.setAttribute("width", window.innerWidth);
    svg_ss.setAttribute("height", window.innerHeight);
    g_svg.setAttribute(
        "transform",
        "translate(0," + window.innerHeight + ") scale(1,-1)"
    );
    CONFIG_DYNAMIC.camera_unit_screen_min = (window.innerWidth - CONFIG.margin) / (CONFIG.x_max + 1);
    CONFIG_DYNAMIC.camera_unit_screen_max = Math.min(window.innerWidth, window.innerHeight) - 30;
}
window.addEventListener("resize", windowResize);

/************************************************************************
 *                            camera
 ***********************************************************************/

/** 2d Vector */
class Vector {
    constructor(x, y) {
        this.x = x || 0;
        this.y = y || 0;
    }
    add(v) {
        return new Vector(this.x + v.x, this.y + v.y);
    }
    addV2(x, y) {
        return new Vector(this.x + x, this.y + y);
    }
    sub(v) {
        return new Vector(this.x - v.x, this.y - v.y);
    }
    mul(r) {
        return new Vector(this.x * r, this.y * r);
    }
    dist(v) {
        return Math.sqrt(
            (this.x - v.x) * (this.x - v.x) + (this.y - v.y) * (this.y - v.y)
        );
    }
    interpolate(v, t) {
        return new Vector(this.x * (1 - t) + v.x * t, this.y * (1 - t) + v.y * t);
    }
}

/** Clip value `x` by [min_, max_]. */
function clip(x, min_, max_) {
    if (x < min_) {
        return min_;
    } else if (x > max_) {
        return max_;
    } else {
        return x;
    }
}

const camera = {
    /* The svg length of the world unit */
    unit_svg: CONFIG_DYNAMIC.camera_unit_screen_init,
    /* The svg position of the world origin */
    o_svg: new Vector(
        CONFIG.margin + 0.5 * CONFIG_DYNAMIC.camera_unit_screen_init,
        CONFIG.margin + 0.5 * CONFIG_DYNAMIC.camera_unit_screen_init
    ),
    zoom: function (pivotSvg /* Vector */, rate) {
        let unit_svg1 = clip(
            this.unit_svg * rate,
            CONFIG_DYNAMIC.camera_unit_screen_min,
            CONFIG_DYNAMIC.camera_unit_screen_max
        );
        let rate1 = unit_svg1 / this.unit_svg;
        this.unit_svg = unit_svg1;

        let origin_sp1 = pivotSvg.add(this.o_svg.sub(pivotSvg).mul(rate1));
        let x_min = window.innerWidth - (CONFIG.x_max + 0.5) * this.unit_svg;
        let x_max = CONFIG.margin + 0.5 * this.unit_svg;
        let y_min = window.innerHeight - (CONFIG.y_max + 0.5) * this.unit_svg;
        let y_max = CONFIG.margin + 0.5 * this.unit_svg;
        if (y_min > y_max) { y_min = y_max; }
        this.o_svg = new Vector(
            clip(origin_sp1.x, x_min, x_max),
            clip(origin_sp1.y, y_min, y_max)
        );

        camera.setTransform();
        updateAxisLabels();
    },
    translate: function (deltaSvg /* Vector */) {
        origin_sp1 = this.o_svg.add(deltaSvg);
        let x_min = window.innerWidth - (CONFIG.x_max + 0.5) * this.unit_svg;
        let x_max = CONFIG.margin + 0.5 * this.unit_svg;
        let y_min = window.innerHeight - (CONFIG.y_max + 0.5) * this.unit_svg;
        let y_max = CONFIG.margin + 0.5 * this.unit_svg;
        if (y_min > y_max) { y_min = y_max; }
        this.o_svg = new Vector(
            clip(origin_sp1.x, x_min, x_max),
            clip(origin_sp1.y, y_min, y_max)
        );

        camera.setTransform();
        updateAxisLabels();
    },
    world2svg: function (ptWorld /* Vector */) {
        return this.o_svg.add(ptWorld.mul(this.unit_svg));
    },
    svg2world: function (ptSvg /* Vector */) {
        return ptSvg.sub(this.o_svg).mul(1 / this.unit_svg);
    },
    flip: function (ptScreen /* Vector */) {
        return new Vector(ptScreen.x, window.innerHeight - ptScreen.y);
    },
    world2svg_v2: function (wpx, wpy) {
        let world_pos = new Vector(wpx, wpy);
        return this.o_svg.add(world_pos.mul(this.unit_svg));
    },
    svg2world_v2: function (spx, spy) {
        let screen_pos = new Vector(spx, spy);
        return screen_pos.sub(this.o_svg).mul(1 / this.unit_svg);
    },
    setTransform: function () {
        g_plot.setAttribute("transform", `translate(${this.o_svg.x},${this.o_svg.y}) scale(${this.unit_svg})`);
    },
};

camera.setTransform();

function getAxisNumber(x) {
    if (MODE === "map") {
        if (x < DATA_JSON.sep_right)
            return x - DATA_JSON["cw2"].shift;
        else
            return x - 1;
    }
    else if (MODE === "cofseq") {
        return Math.floor(x / 3) + CONFIG.x_min_cofseq[x % 3];
    }
    else {
        return x;
    }
}

function updateAxisLabels() {
    var stepLabel = Math.ceil(CONFIG.axis_text_sep_screen / camera.unit_svg);
    if (MODE === "cofseq") {
        if (stepLabel > 3)
            stepLabel = Math.floor(stepLabel / 3) * 3;
        else if (stepLabel == 2)
            stepLabel = 3;
    }
    let i_min = Math.ceil(camera.svg2world(new Vector(30, 0)).x / stepLabel) * stepLabel;
    let i_max = Math.floor(camera.svg2world(new Vector(window.innerWidth, 0)).x);
    g_xaxis.innerHTML = "";
    for (let i = i_min; i <= i_max; i += stepLabel) {
        let xText = camera.world2svg(new Vector(i, 0)).x;
        let label = `<text x="${xText}" y="-10">${getAxisNumber(i)}</text>\n`;
        g_xaxis.insertAdjacentHTML("beforeend", label);
    }
    i_min = Math.ceil(camera.svg2world(new Vector(0, 30)).y / stepLabel) * stepLabel;
    i_max = Math.floor(camera.svg2world(new Vector(0, window.innerHeight)).y);
    g_yaxis.innerHTML = "";
    for (let i = i_min; i <= i_max; i += stepLabel) {
        let yText = camera.world2svg(new Vector(0, i)).y;
        let label = `<text x="26" y="${-yText}" dy="0.25em">${i}</text>\n`;
        g_yaxis.insertAdjacentHTML("beforeend", label);
    }
}

function latexMon(mon, data_json) {
    if (mon.length === 0) {
        return "1";
    }
    else {
        result = "";
        const gen_names = data_json["gen_names"];
        for (let i = 0; i < mon.length - 1; i += 2) {
            if (mon[i + 1] == 1) {
                result += `${gen_names[mon[i]]}`;
            }
            else if (mon[i + 1] < 10) {
                result += `${gen_names[mon[i]]}^${mon[i + 1]}`;
            }
            else {
                result += `${gen_names[mon[i]]}^{${mon[i + 1]}}`;
            }
        }
        if (mon.length % 2 === 1) {
            result += `${data_json["v_names"][mon[mon.length - 1]]}`;
        }
        return result;
    }
}

function getJsonForBullet(bullet) {
    if (bullet.classList.contains("cw")) {
        return DATA_JSON;
    }
    else if (bullet.classList.contains("cw1")) {
        return DATA_JSON["cw1"];
    }
    else if (bullet.classList.contains("cw2")) {
        return DATA_JSON["cw2"];
    }
    else if (bullet.classList.contains("cs0")) {
        return DATA_JSON["cofseq_groups"][0];
    }
    else if (bullet.classList.contains("cs1")) {
        return DATA_JSON["cofseq_groups"][1];
    }
    else if (bullet.classList.contains("cs2")) {
        return DATA_JSON["cofseq_groups"][2];
    }
    else{
        console.log("Error in getJsonForBullet: ", bullet);
    }
}

function latexBullet(bullet) {
    const data_json = getJsonForBullet(bullet);
    const bullet_json = data_json["bullets"][bullet.dataset.i];
    const basis_json = data_json["basis"];
    let str_base = "";
    const offset_X = parseInt(bullet_json["i0"]);
    for (let i = 0; i < bullet_json["b"].length; ++i) {
        if (i > 0) {
            str_base += "+";
        }
        str_base += latexMon(basis_json[offset_X + parseInt(bullet_json["b"][i])], data_json);
    }
    return str_base;
}

function replaceAll(str, find, replace) {
    return str.replace(new RegExp(find, 'g'), replace);
}

function addBulletLabels() {
    const bullets = document.getElementsByClassName("b");
    g_labels.innerHTML = "";
    for (const bullet of bullets) {
        if (bullet.tagName === "circle" && bullet.id.slice(0, 1) === "b") {
            let str_mon = latexBullet(bullet.id);
            str_mon = replaceAll(str_mon, /\\Delta\s?/, "Δ");
            str_mon = replaceAll(str_mon, /\\mu\s?/, "μ");
            str_mon = replaceAll(str_mon, /\\rho\s?/, "ρ");
            str_mon = replaceAll(str_mon, /\\iota\s?/, "ι");
            str_mon = replaceAll(str_mon, /\\eta\s?/, "η");
            str_mon = replaceAll(str_mon, /\\nu\s?/, "ν");
            str_mon = replaceAll(str_mon, /\\bar\\sigma\s?/, "σ&#772");
            str_mon = replaceAll(str_mon, /\\sigma\s?/, "σ");
            str_mon = replaceAll(str_mon, /\\epsilon\s?/, "ε");
            str_mon = replaceAll(str_mon, /\\theta\s?/, "θ");
            str_mon = replaceAll(str_mon, /\\bar\\kappa\s?/, "κ&#772");
            str_mon = replaceAll(str_mon, /\\kappa\s?/, "κ");
            str_mon = replaceAll(str_mon, /\\zeta\s?/, "ζ");
            str_mon = replaceAll(str_mon, /\^\\prime\s?/, "'");
            str_mon = replaceAll(str_mon, /\^\{\\prime\\prime}\s?/, "''");
            str_mon = replaceAll(str_mon, /_(\d)/, "$1");
            str_mon = replaceAll(str_mon, /_\{(\d\d)\}/, "$1");

            const re = /^(?!(P|P\^\d|P\^\d\d|Δ|M|M_1|\()h(0|1|2)).*h(0|1|2)/;
            if (str_mon.length <= 10 && parseFloat(bullet.getAttribute("cx")) <= 127 && !(str_mon.match(re) && str_mon.length > 2)) {
                const r = parseFloat(bullet.getAttribute("r"));
                let class_ = "label";
                if (bullet.classList.contains("b0"))
                    class_ += " label0";
                else if (bullet.classList.contains("b1"))
                    class_ += " label1";
                const label = `<text class="${class_}" x=${parseFloat(bullet.getAttribute("cx")) - r * 0.75} y=${-parseFloat(bullet.getAttribute("cy")) + r * 2} font-size=${r * 1.125} data-page=${bullet.dataset.page}>${str_mon}</text>\n`;
                g_labels.insertAdjacentHTML("beforeend", label);
            }
        }
    }
}

function addRects() {
    if (MODE === "ss") { // products
        let i = 0;
        for (const deg of DATA_JSON['degs_factors']) {
            let rect_product = `<rect id="rect_prod${i++}" x="-1000" y="-1000" width="1" height="1" fill="green" opacity="0.1"  data-x="${deg[0]}" data-y="${deg[1]}" />`;
            g_plot.insertAdjacentHTML("afterbegin", rect_product);
        }
    }
    if (MODE === "cofseq") { // color separators
        for (let i = 0; i < CONFIG.x_max / 3; i += 2) {
            let rect_product = `<rect x="${3 * i - 0.5}" y="-0.5" width="3" height="300" fill="#dddddd"/>`;
            g_plot.insertAdjacentHTML("afterbegin", rect_product);
        }
    }
}

/************************************************************************
 *                       Pointer events
 ************************************************************************/

/* pointers */
const pointerCache = new Array();
var prevPtsDist = null;
var prevPt = null;
var prevPinchScale = null;

/* other globals */
var timerClearCache = null;

function getDistPts() {
    let p1Screen = new Vector(pointerCache[0].offsetX, pointerCache[0].offsetY);
    let p2Screen = new Vector(pointerCache[1].offsetX, pointerCache[1].offsetY);
    return p1Screen.dist(p2Screen);
}

function restartTimer() {
    clearTimeout(timerClearCache);
    timerClearCache = window.setTimeout(function () { pointerCache.length = 0; }, 3000);
}

function on_pointerdown(event) {
    if (event.button === 0) {
        div_menu_style.visibility = "hidden";
        div_binfo_style.visibility = "hidden";
        if (event.shiftKey && "sep_right" in DATA_JSON) {
            DATA_JSON.sep_right = Math.ceil(camera.svg2world_v2(event.offsetX, 0).x);
            updateVisibility();
            updateAxisLabels();
        }

        /* This event is cached to support 2-finger gestures */
        pointerCache.push(event);

        if (pointerCache.length === 1) {
            prevPt = new Vector(event.offsetX, event.offsetY);
        }
        else if (pointerCache.length === 2) {
            prevPtsDist = getDistPts();
        }
        restartTimer();
    }
}

/* This function implements a 2-pointer horizontal pinch/zoom gesture. */
function on_pointermove(event) {
    /* Check if this event is in the cache and update it */
    let index = 0;
    for (; index < pointerCache.length; index++) {
        if (event.pointerId === pointerCache[index].pointerId) {
            pointerCache[index] = event;
            break;
        }
    }

    /* Move only when the only one down pointer moves */
    if (pointerCache.length === 1 && index < pointerCache.length) {
        let curPt = new Vector(event.offsetX, event.offsetY);
        let deltaScreen = curPt.sub(prevPt);
        camera.translate(new Vector(deltaScreen.x, -deltaScreen.y));
        prevPt = curPt;
    }

    /* If two pointers are down, check for pinch gestures */
    if (pointerCache.length === 2 && index < pointerCache.length) {
        let p1Svg = camera.flip(new Vector(pointerCache[0].offsetX, pointerCache[0].offsetY));
        let p2Svg = camera.flip(new Vector(pointerCache[1].offsetX, pointerCache[1].offsetY));
        let curDist = p1Svg.dist(p2Svg);
        camera.zoom(index === 0 ? p2Svg : p1Svg, curDist / prevPtsDist);
        prevPtsDist = curDist;
    }
}

/**
 * Return if at least one event is removed
 */
function removeEvent(event_id) {
    /* Remove this event from the target's cache */
    for (let i = 0; i < pointerCache.length; i++) {
        if (pointerCache[i].pointerId === event_id) {
            pointerCache.splice(i, 1);
            return true;
        }
    }
    return false;
}

var bullet_selected = null;

function select_bullet(bullet) {
    bullet_selected = bullet;
    bullet_selected.setAttribute("fill", "red");
    rect_selected.setAttribute("x", Math.round(bullet.getAttribute("cx")) - 0.5);
    rect_selected.setAttribute("y", Math.round(bullet.getAttribute("cy")) - 0.5);

    g_prod.innerHTML = "";
    g_map.innerHTML = "";
    // products
    if (MODE == "ss") {
        prods = DATA_JSON["prods"][bullet.dataset.i];
        for (const j in DATA_JSON["degs_factors"]) {
            let rect_prod = document.getElementById(`rect_prod${j}`);
            rect_prod.setAttribute('x', parseFloat(rect_selected.getAttribute("x")) + parseFloat(rect_prod.dataset.x));
            rect_prod.setAttribute('y', parseFloat(rect_selected.getAttribute("y")) + parseFloat(rect_prod.dataset.y));
        }
        for (const j in prods) {
            for (const i of prods[j]['p']) {
                const bullet2 = DATA_JSON["bullets"][i];
                const circle_prod = `<circle class="p" cx="${bullet2.x}" cy="${bullet2.y}" r="${bullet2['r'] * 1.7}", data-i=${i}></circle>`;
                g_prod.insertAdjacentHTML("beforeend", circle_prod);
            }
        }
    }

    // images of maps
    if (bullet.classList.contains("cw1") && Math.round(bullet.getAttribute("cx")) == DATA_JSON.sep_right && bullet.dataset.i in DATA_JSON["maps"]) {
        for (const i of DATA_JSON["maps"][bullet.dataset.i]) {
            const bullet2 = DATA_JSON["cw2"]["bullets"][i];
            const circle_image = `<circle class="p baux cw2" cx="${bullet2.x + DATA_JSON["cw2"].shift}" cy="${bullet2.y}" r="${bullet2['r'] * 1.7}", data-i=${i}></circle>`;
            g_map.insertAdjacentHTML("beforeend", circle_image);
        }
    }
}

function on_pointerup(event) {
    /* Remove this pointer from the cache */
    if (event.button === 0) {
        if (removeEvent(event.pointerId)) {
            if (pointerCache.length === 0) {
                prevPt = null;
            }
            else if (pointerCache.length === 1) {
                prevPt = new Vector(pointerCache[0].offsetX, pointerCache[0].offsetY);
            }
            else if (pointerCache.length === 2) {
                prevPtsDist = getDistPts();
            }
        }

        const bullet = event.target;
        if (bullet == bullet_selected) {
            /* Info pane */
            const posX = event.clientX;
            const posY = window.innerHeight - event.clientY;

            div_binfo_style.left = posX + "px";
            div_binfo_style.bottom = posY + "px";
            div_binfo_style.visibility = "visible";

            const bullet_json = getJsonForBullet(bullet)["bullets"][bullet.dataset.i];

            p_deg.innerHTML = `Deg: (${getAxisNumber(Math.round(bullet.getAttribute("cx")))},${Math.round(bullet.getAttribute("cy"))})`;
            p_base.innerHTML = `Basis: ${bullet_json['b']}`;
            const str_base = latexBullet(bullet);
            const tex_base = katex.renderToString(str_base, { throwOnError: false });
            p_latex.innerHTML = `LaTeX: ${tex_base}`;

            const level = bullet_json['l'];
            if (level === 5000) { p_diff.innerHTML = `Permanent`; }
            else if (level === 10000 - CONFIG.R_PERM) { p_diff.innerHTML = `Permanent`; }
            else if (level > 10000 - CONFIG.R_PERM) {
                const r = 10000 - level;
                let str_diff = `d_{${r}}(\\mathrm{this})=(${bullet_json['d']})`;
                str_diff = str_diff.replace('null', '?');
                p_diff.innerHTML = katex.renderToString(str_diff, { throwOnError: false });
            }
            else {
                const r = level;
                let str_diff = `d_{${r}}(${bullet_json['d']})=\\mathrm{this}`;
                str_diff = str_diff.replace('null', '?');
                p_diff.innerHTML = katex.renderToString(str_diff, { throwOnError: false });
            }
        }
        else if (bullet.classList.contains("b")) {
            if (bullet_selected !== null) {
                if (bullet_selected.getAttribute("stroke-width") !== null) {
                    bullet_selected.setAttribute("fill", "transparent");
                }
                else {
                    bullet_selected.removeAttribute("fill");
                }
                bullet_selected = null;
            }
            select_bullet(bullet);
        }

        restartTimer();
    }
}

function on_wheel(event) {
    let pivotScreen = new Vector(event.offsetX, event.offsetY);
    let pivotSvg = camera.flip(pivotScreen);
    camera.zoom(pivotSvg, event.deltaY < 0 ? CONFIG.camera_zoom_rate : 1 / CONFIG.camera_zoom_rate);
    event.preventDefault();
}

/* For macbook trackpad pinch gesture */
function on_pinch(event) {
    let pivotScreen = new Vector(event.clientX, event.clientY);
    let pivotSvg = camera.flip(pivotScreen);
    camera.zoom(pivotSvg, event.scale / prevPinchScale);
    prevPinchScale = event.scale;
    event.preventDefault();
}

function on_key_down(event) {
    //console.log(event.which)
    if (!event.shiftKey) {
        if (event.which === 39) { /* Right */
            camera.translate(new Vector(-CONFIG.camera_translate_pixels, 0));
        }
        else if (event.which === 37) { /* Left */
            camera.translate(new Vector(CONFIG.camera_translate_pixels, 0));
        }
        else if (event.which === 38) { /* Up */
            camera.translate(new Vector(0, -CONFIG.camera_translate_pixels));
        }
        else if (event.which === 40) { /* Down */
            camera.translate(new Vector(0, CONFIG.camera_translate_pixels));
        }
        else if (event.which === 189) { /* - */
            const pivotSvg = new Vector(window.innerWidth / 2, window.innerHeight / 2);
            camera.zoom(pivotSvg, 1 / CONFIG.camera_zoom_rate);
        }
        else if (event.which === 187) { /* = */
            const pivotSvg = new Vector(window.innerWidth / 2, window.innerHeight / 2);
            camera.zoom(pivotSvg, CONFIG.camera_zoom_rate);
        }
    }
    if ("from" in DATA_JSON && event.shiftKey) {
        if (event.which === 39) { // Right arrow
            if (DATA_JSON.sep_right < CONFIG.x_max) {
                DATA_JSON.sep_right += 1;
                updateVisibility();
                updateAxisLabels();
            }
        }
        else if (event.which === 37) { // Left arrow
            if (DATA_JSON.sep_right > 1) {
                DATA_JSON.sep_right -= 1;
                updateVisibility();
                updateAxisLabels();
            }
        }
    }
}

function on_pointerenter_bullet(event) {
    let tgt = event.target;
    circle_mouseon.setAttribute("cx", tgt.getAttribute("cx"));
    circle_mouseon.setAttribute("cy", tgt.getAttribute("cy"));
    circle_mouseon.setAttribute("r", Number(tgt.getAttribute("r")) * 1.3);
}
function on_pointerleave_bullet(event) {
    let tgt = event.target;
    circle_mouseon.setAttribute("cx", "-1000");
}

/************************************
 * Menu events handlers 
 ************************************/

function on_contextmenu(event) {
    if (!event.ctrlKey) {
        let posX = event.clientX;
        let posY = event.clientY;

        if (event.target.id === "button_cm") {
            div_menu_style.left = null;
            div_menu_style.right = (window.innerWidth - posX) + "px";
        }
        else {
            div_menu_style.left = posX + "px";
            div_menu_style.right = null;
        }
        div_menu_style.top = posY + "px";
        div_menu_style.visibility = "visible";

        event.preventDefault();
    }
}

function on_click_about() {
    alert("Author: Weinan Lin");
}

function on_time() {
    alert(`Time: ${DATA_JSON.time}`);
}

function updateVisibility() {
    if (MODE == "map") {
        var sep_right = DATA_JSON.sep_right;
        var sep_left = sep_right - 1;
        rect_separator.setAttribute('x', sep_left);
        rect_second_ss.setAttribute('x', sep_left - 300);
    }
    for (const ele of document.getElementsByClassName("p")) {
        const classList = ele.classList;
        if (!classList.contains("bp"))
            ele.style.visibility = "hidden";
        if (classList.contains("b") || classList.contains("baux")) {
            if ((!classList.contains("cw1") || Math.round(ele.getAttribute("cx")) >= sep_right) && (!classList.contains("cw2") || Math.round(ele.getAttribute("cx")) <= sep_left)) {
                const data_json = getJsonForBullet(ele);
                const bullet_json = data_json["bullets"][ele.dataset.i];
                if (bullet_json['p'] >= CONFIG_DYNAMIC.page) {
                    ele.removeAttribute("style");
                    if (bullet_json['p'] == CONFIG.R_PERM && bullet_json['d'] !== null) {
                        if (bullet_json['l'] > 10000 - CONFIG_DYNAMIC.page) {
                            ele.setAttribute("fill", "transparent");
                            ele.setAttribute("stroke", bullet_json['c']);
                            ele.setAttribute("stroke-width", bullet_json['r'] / 3);
                        }
                        else {
                            ele.removeAttribute("fill");
                            ele.removeAttribute("stroke");
                            ele.removeAttribute("stroke-width");
                        }
                    }
                }
            }
        }
        else {
            if (!classList.contains("cw1") || (Math.round(ele.getAttribute("x1")) >= sep_right && Math.round(ele.getAttribute("x2")) >= sep_right)) {
                if (!classList.contains("cw2") || (Math.round(ele.getAttribute("x1")) <= sep_left && Math.round(ele.getAttribute("x2")) <= sep_left)) {
                    if (classList.contains("sl")) {
                        if (ele.dataset.page >= CONFIG_DYNAMIC.page) {
                            ele.removeAttribute("style");
                        }
                    }
                    else if (classList.contains("ml")) {
                        if (Math.round(ele.getAttribute("x1")) == sep_right && ele.dataset.page >= CONFIG_DYNAMIC.page) {
                            ele.removeAttribute("style");
                        }
                    }
                    else if (classList.contains("dl")) {
                        if (ele.dataset.page == CONFIG_DYNAMIC.page || (CONFIG_DYNAMIC.showLines === "All diff" && ele.dataset.page > CONFIG_DYNAMIC.page)) {
                            ele.removeAttribute("style");
                        }
                    }
                    else if (classList.contains("nd")) {
                        if (ele.dataset.r <= CONFIG_DYNAMIC.dashedLevel && (CONFIG_DYNAMIC.showLines === "All diff" || ele.dataset.r <= CONFIG_DYNAMIC.page)) {
                            ele.removeAttribute("style");
                        }
                    }
                    else if (classList.contains("lb")) { // labels
                        if (ele.dataset.page >= CONFIG_DYNAMIC.page) {
                            ele.removeAttribute("style");
                        }
                    }
                }
            }
        }
    }
}

const select_page_map = {
    "E0": 0,
    "E1": 1,
    "E2": 2,
    "E3": 3,
    "E4": 4,
    "E5": 5,
    "E6": 6,
    "Einf": CONFIG.R_PERM,
};

function on_select_page(event) {
    CONFIG_DYNAMIC.page = select_page_map[event.target.value];
    updateVisibility();
}

const select_dashed_map = {
    "dash none": 0,
    "dash d2": 2,
    "dash d3": 3,
    "dash d4": 4,
    "dash d5": 5,
    "dash d6": 6,
    "dash all": CONFIG.R_PERM,
};

function on_select_dashed(event) {
    CONFIG_DYNAMIC.dashedLevel = select_dashed_map[event.target.value];
    updateVisibility();
}

function on_select_lines(event) {
    CONFIG_DYNAMIC.showLines = event.target.value
    updateVisibility();
}

/*****************************************************************************
 *                   Initialization of event handlers
 *****************************************************************************/
function initHandlers() {
    svg_ss.addEventListener("wheel", on_wheel);
    svg_ss.addEventListener("pointerdown", on_pointerdown);
    svg_ss.addEventListener("pointermove", on_pointermove);
    svg_ss.addEventListener("pointerup", on_pointerup);
    svg_ss.addEventListener("pointerleave", on_pointerup);

    svg_ss.addEventListener("contextmenu", on_contextmenu);
    document.addEventListener("keydown", on_key_down);

    const div_menu = document.getElementById("div_menu");
    div_menu.onclick = event => { div_menu_style.visibility = "hidden"; };


    /* For macbook */
    if (navigator.userAgent.match("Macintosh")) {
        window.addEventListener("gesturestart", event => { prevPinchScale = 1.0; event.preventDefault(); });
        window.addEventListener("gesturechange", on_pinch);
        window.addEventListener("gestureend", event => event.preventDefault());
    }

    if (navigator.userAgent.match("Macintosh")) {
        CONFIG.camera_zoom_rate = 1.06;
    }

    let str_text_date = `<text id="text8733d2c" x="60" y="-40" opacity="0.5" transform="scale(1,-1)" style="-moz-user-select: none;-webkit-user-select: none;-ms-user-select: none;user-select: none;-o-user-select: none;">js:07/29</text>`;
    g_yaxis.insertAdjacentHTML("afterend", str_text_date);
    const text_date = document.getElementById('text8733d2c');
    window.setTimeout(function () { text_date.remove(); }, 5000);
}



/************************************************************************
 *                         Plot svg by json
 ***********************************************************************/

function loadPlot(data_json) {
    const xshift = "shift" in data_json ? data_json.shift : 0;
    const xfactor = "factor" in data_json ? data_json.factor : 1;
    const trans = (x) => ((x - Math.round(x)) + Math.round(x) * xfactor + xshift);
    for (const batchEnd = data_json.iPlotB + CONFIG.plot_batchSize; data_json.iPlotB < batchEnd && data_json.iPlotB < data_json["bullets"].length; data_json.iPlotB++) {
        const bullet = data_json["bullets"][data_json.iPlotB];
        const ele_bullet = `<circle data-i="${data_json.iPlotB}" class="p b ${data_json.class}" cx="${trans(bullet.x)}" cy="${bullet.y}" r="${bullet.r}"> </circle>`;
        g_bullets[bullet['c']].insertAdjacentHTML("beforeend", ele_bullet);
    }
    if (data_json["type"] === "cofseq_gp" && "bullets_p" in data_json) {
        for (const batchEnd = data_json.iPlotBP + CONFIG.plot_batchSize; data_json.iPlotBP < batchEnd && data_json.iPlotBP < data_json["bullets_p"].length; data_json.iPlotBP++) {
            const bullet = data_json["bullets_p"][data_json.iPlotBP];
            const ele_bullet = `<circle class="p bp ${data_json.class}" cx="${trans(bullet.x)}" cy="${bullet.y}" r="${bullet.r * 0.75}" fill="transparent" stroke="${bullet.c}" stroke-width="${bullet.r / 2}"> </circle>`;
            g_bullets[bullet['c']].insertAdjacentHTML("beforeend", ele_bullet);
        }
    }
    let keys_prods = Object.keys(data_json["prods"]);
    for (const batchEnd = data_json.iPlotSL + CONFIG.plot_batchSize / 2; data_json.iPlotSL < batchEnd && data_json.iPlotSL < keys_prods.length; data_json.iPlotSL++) {
        const lines = data_json["prods"][keys_prods[data_json.iPlotSL]];
        for (const line of lines) {
            if (line['l'] == 0) {
                continue;
            }
            const bullet1 = data_json["bullets"][keys_prods[data_json.iPlotSL]];
            for (const i of line["p"]) {
                const bullet2 = data_json["bullets"][i];
                const width = Math.min(bullet1['r'], bullet2['r']) / 4;
                const page = Math.min(bullet1['p'], bullet2['p']);
                const ele_line = `<line class="p sl ${data_json.class}" x1="${trans(bullet1.x)}" y1="${bullet1.y}" x2="${trans(bullet2.x)}" y2="${bullet2.y}" stroke="black" stroke-width="${width}" data-page="${page}"> </line>`;
                g_strtlines.insertAdjacentHTML("beforeend", ele_line);
            }
        }
    }
    if (data_json["type"] !== "cofseq_gp") {
        for (const batchEnd = data_json.iPlotDL + CONFIG.plot_batchSize / 4; data_json.iPlotDL < batchEnd && data_json.iPlotDL < data_json["diffs"].length; data_json.iPlotDL++) {
            const diff = data_json["diffs"][data_json.iPlotDL];
            const bullet1 = data_json["bullets"][diff["i"]];
            for (const j of diff["j"]) {
                const bullet2 = data_json["bullets"][j];
                const width = Math.min(bullet1['r'], bullet2['r']) / 4;
                const page = Math.min(bullet1['p'], bullet2['p']);
                const ele_line = `<line class="p dl ${data_json.class}" x1="${trans(bullet1.x)}" y1="${bullet1.y}" x2="${trans(bullet2.x)}" y2="${bullet2.y}" stroke-width="${width}" data-page="${page}"> </line>`;
                g_diff_lines[diff["r"] <= 6 ? diff["r"] : 6].insertAdjacentHTML("beforeend", ele_line);
            }
        }
    } 
    else {
        const trans2 = (x, nx) => ((x - Math.round(x)) + nx);
        for (const batchEnd = data_json.iPlotDL + CONFIG.plot_batchSize / 4; data_json.iPlotDL < batchEnd && data_json.iPlotDL < data_json["diffs"].length; data_json.iPlotDL++) {
            const diff = data_json["diffs"][data_json.iPlotDL];
            const bullet1 = data_json["bullets"][diff["i"]];
            const x1 = trans(bullet1.x);
            const nx2 = Math.round(x1) - 1;
            for (const j of diff["j"]) {
                const bullet2 = data_json["bullets2"][j];
                const width = Math.min(bullet1['r'], bullet2['r']) / 4;
                const page = Math.min(bullet1['p'], bullet2['p']);
                const ele_line = `<line class="p dl ${data_json.class}" x1="${x1}" y1="${bullet1.y}" x2="${trans2(bullet2.x, nx2)}" y2="${bullet2.y}" stroke-width="${width}" data-page="${page}"> </line>`;
                g_diff_lines[diff["r"] <= 6 ? diff["r"] : 6].insertAdjacentHTML("beforeend", ele_line);
            }
        }
    }
    for (const batchEnd = data_json.iPlotND + CONFIG.plot_batchSize / 4; data_json.iPlotND < batchEnd && data_json.iPlotND < data_json["nds"].length; data_json.iPlotND++) {
        const nd = data_json["nds"][data_json.iPlotND];
        const bullet1 = data_json["bullets"][nd["i"]];
        const width = bullet1['r'] / 4;
        const ele_line = `<line class="p nd ${data_json.class}" x1="${trans(bullet1.x)}" y1="${bullet1.y}" x2="${trans(Math.round(bullet1.x)) - 1}" y2="${Math.round(bullet1.y) + nd['r']}" stroke-width="${width}" stroke-dasharray="0.1,0.1" data-r="${nd['r']}"> </line>`;
        g_diff_lines[nd["r"] <= 6 ? nd["r"] : 6].insertAdjacentHTML("beforeend", ele_line);
    }
    if (data_json.iPlotB < data_json["bullets"].length || data_json.iPlotSL < keys_prods.length || data_json.iPlotDL < data_json["diffs"].length || data_json.iPlotND < data_json["nds"].length || ("bullets_p" in data_json && data_json.iPlotBP < data_json["bullets_p"].length)) {
        window.requestAnimationFrame(() => loadPlot(data_json));
    }
    else {
        /* Desktop browser will support pointer enter and leave events */
        if (navigator.userAgent.match("Windows") || navigator.userAgent.match("Macintosh")) {
            const bullets = document.getElementsByClassName("b");

            for (const b of bullets) {
                b.onpointerenter = on_pointerenter_bullet;
                b.onpointerleave = on_pointerleave_bullet;
            }
        }
        updateVisibility();
        updateAxisLabels();
    }
}

function loadPlotMap(data_json) {
    const shift2 = data_json["cw2"].shift;
    const map_keys = Object.keys(data_json["maps"]);
    for (const batchEnd = data_json.iPlotMap + CONFIG.plot_batchSize; data_json.iPlotMap < batchEnd && data_json.iPlotMap < map_keys.length; data_json.iPlotMap++) {
        const key = map_keys[data_json.iPlotMap];
        const bullet1 = data_json["cw1"]["bullets"][key];
        for (const i of data_json["maps"][key]) {
            const bullet2 = data_json["cw2"]["bullets"][i];
            const width = Math.min(bullet1['r'], bullet2['r']) / 4;
            const page = Math.min(bullet1['p'], bullet2['p']);
            const ele_line = `<line class="p ml" x1="${bullet1.x + 1}" y1="${bullet1.y}" x2="${bullet2.x + shift2}" y2="${bullet2.y}" stroke-width="${width}" data-page="${page}"> </line>`;
            g_strtlines_purple.insertAdjacentHTML("beforeend", ele_line);
        }
    }
    if (data_json.iPlotMap < map_keys.length) {
        window.requestAnimationFrame(() => loadPlotMap(data_json));
    }
    else {
        updateVisibility();
    }
}

function Plot(data_json) {
    if (data_json["type"] === "map") {
        data_json.iPlotMap = 0;
        window.requestAnimationFrame(() => loadPlotMap(data_json));
    }
    else if (["ring", "module", "cofseq_gp"].includes(data_json["type"])) {
        data_json.iPlotB = 0; /* bullets */
        data_json.iPlotSL = 0; /* structure lines */
        data_json.iPlotDL = 0; /* diff lines */
        data_json.iPlotND = 0; /* null differential lines */
        if (data_json["type"] == "cofseq_gp")
            data_json.iPlotBP = 0; /* bullets_p */
        window.requestAnimationFrame(() => loadPlot(data_json));
    }
}

const LOADED_SCRIPTS = new Set();
function waitUntil(checkFlag, doSomething) {
    if (checkFlag() === false) {
        window.setTimeout(() => { waitUntil(checkFlag, doSomething) }, 200); /* this checks the flag every 100 milliseconds*/
    } else {

        doSomething();
    }
}
function loadScript(path, on_load) {
    if (LOADED_SCRIPTS.has(path)) {
        waitUntil(() => { return LOADED_SCRIPTS.has(path + "_done") }, on_load);
    }
    else {
        LOADED_SCRIPTS.add(path);
        const scriptEle = document.createElement("script");
        scriptEle.setAttribute("src", path);
        document.body.appendChild(scriptEle);
        scriptEle.addEventListener("load", () => { on_load(); LOADED_SCRIPTS.add(path + "_done"); });
    }
}

function processParams() {
    const dir = urlParams.get("diagram") || "mix";
    const data = DATA_NAME;
    document.getElementById("title").innerHTML = `${data}: AdamsSS`;

    /* Adjust camera */
    if (urlParams.get("scale") !== null) {
        const pivot_svg = camera.world2svg(new Vector(urlParams.get("x"), urlParams.get("y")));
        const scale = Number(urlParams.get("scale"));
        camera.zoom(pivot_svg, scale);
    }
    if (urlParams.get("x") !== null) {
        const wx = Number(urlParams.get("x")) + (data.includes("__") ? 1 : 0);
        const pivot_svg = camera.world2svg(new Vector(wx, 0));
        camera.translate(new Vector(CONFIG.margin + (window.innerWidth - CONFIG.margin) / 2 - pivot_svg.x, 0));
    }
    if (urlParams.get("y") !== null) {
        const wy = Number(urlParams.get("y"));
        const pivot_svg = camera.world2svg(new Vector(0, wy));
        camera.translate(new Vector(0, CONFIG.margin + (window.innerHeight - CONFIG.margin) / 2 - pivot_svg.y));
    }

    /* Add bullets */
    loadScript(`${dir}/${data}.js`, () => {
        DATA_JSON = globalThis[`DATA_JSON_${data}`];
        addRects();
        if (MODE === "ss") {
            DATA_JSON.class = "cw";
            Plot(DATA_JSON);

            /* module */
            if ("over" in DATA_JSON) {
                loadScript(`${dir}/${DATA_JSON["over"]}.js`, () => {
                    DATA_JSON["gen_names"] = globalThis[`DATA_JSON_${DATA_JSON["over"]}`]["gen_names"];
                });
            }
        }
        /* map */
        else if (MODE === "map") {
            DATA_JSON.sep_right = 1;
            loadScript(`${dir}/${DATA_JSON["from"]}.js`, () => {
                DATA_JSON["cw1"] = Object.assign({}, globalThis[`DATA_JSON_${DATA_JSON["from"]}`]);
                DATA_JSON["cw1"].class = "cw1";
                DATA_JSON["cw1"].shift = 1;
                Plot(DATA_JSON["cw1"]);

                loadScript(`${dir}/${DATA_JSON["to"]}.js`, () => {
                    DATA_JSON["cw2"] = Object.assign({}, globalThis[`DATA_JSON_${DATA_JSON["to"]}`]);
                    DATA_JSON["cw2"].class = "cw2";
                    DATA_JSON["cw2"].shift = DATA_JSON["sus"];
                    Plot(DATA_JSON["cw2"]);
                    Plot(DATA_JSON);

                    /* modules */
                    if ("over" in DATA_JSON["cw1"]) {
                        loadScript(`${dir}/${DATA_JSON["cw1"]["over"]}.js`, () => {
                            DATA_JSON["cw1"]["gen_names"] = globalThis[`DATA_JSON_${DATA_JSON["cw1"]["over"]}`]["gen_names"];

                            if ("over" in DATA_JSON["cw2"]) {
                                loadScript(`${dir}/${DATA_JSON["cw2"]["over"]}.js`, () => {
                                    DATA_JSON["cw2"]["gen_names"] = globalThis[`DATA_JSON_${DATA_JSON["cw2"]["over"]}`]["gen_names"];
                                });
                            }
                        });
                    }
                });
            });
        }
        /* cofseq */
        else if (MODE === "cofseq") {
            const n0 = Math.min(0, -DATA_JSON["degs_maps"][0][0], -DATA_JSON["degs_maps"][0][0] - DATA_JSON["degs_maps"][1][0]);
            CONFIG.x_min_cofseq = [n0 + DATA_JSON["degs_maps"][0][0] + DATA_JSON["degs_maps"][1][0], n0 + DATA_JSON["degs_maps"][0][0], n0];

            for (let iCw = 0; iCw < 3; ++iCw) {
                DATA_JSON["cofseq_groups"][iCw].shift = 2 - iCw - 3 * CONFIG.x_min_cofseq[2 - iCw];
                DATA_JSON["cofseq_groups"][iCw].factor = 3;
                DATA_JSON["cofseq_groups"][iCw].class = "cs" + iCw;
                DATA_JSON["cofseq_groups"][iCw]["bullets2"] = DATA_JSON["cofseq_groups"][(iCw + 1) % 3]["bullets"]
                Plot(DATA_JSON["cofseq_groups"][iCw]);

                loadScript(`${dir}/${DATA_JSON["names"][iCw]}.js`, () => {
                    const data_json_csi = globalThis[`DATA_JSON_${DATA_JSON["names"][iCw]}`];
                    DATA_JSON["cofseq_groups"][iCw]["basis"] = data_json_csi["basis"];
                    if ("gen_names" in data_json_csi)
                        DATA_JSON["cofseq_groups"][iCw]["gen_names"] = data_json_csi["gen_names"];
                    else {
                        DATA_JSON["cofseq_groups"][iCw]["v_names"] = data_json_csi["v_names"];
                        loadScript(`${dir}/${data_json_csi["over"]}.js`, () => {
                            DATA_JSON["cofseq_groups"][iCw]["gen_names"] = globalThis[`DATA_JSON_${data_json_csi["over"]}`]["gen_names"];
                        });
                    }
                });
            }
        }
    });
}

/***************************************************
 *                   initialization
 ***************************************************/
function addGridLines() {
    const g_grid = document.getElementById("g_grid");
    for (let i = 0; i <= CONFIG.y_max_grid; i += 1) {
        const line = `<line x1="-0.5" y1="${i}" x2="${CONFIG.x_max + 0.5}" y2="${i}"></line>\n`;
        g_grid.insertAdjacentHTML("beforeend", line);
    }
    for (let i = 0; i <= CONFIG.x_max; i += 1) {
        const line = `<line x1="${i}" y1="-.5" x2="${i}" y2="${CONFIG.y_max_grid}"></line>\n`;
        g_grid.insertAdjacentHTML("beforeend", line);
    }
}

function init() {
    if (MODE === "cofseq") {
        const options = "<option>E0</option>\n<option>E1</option>\n";
        select_page.insertAdjacentHTML("afterbegin", options);
    }

    addGridLines();
    // if (MODE !== "FromRes")
    //     addBulletLabels();
    updateAxisLabels();

    initHandlers();

    processParams();
}
