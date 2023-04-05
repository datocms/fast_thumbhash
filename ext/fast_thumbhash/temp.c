

// /**
//  * Extracts the average color from a ThumbHash. RGB is not be premultiplied by A.
//  *
//  * @param hash The bytes of the ThumbHash.
//  * @returns The RGBA values for the average color. Each value ranges from 0 to 1.
//  */
// export function thumbHashToAverageRGBA(hash) {
//   let { min, max } = Math
//   let header = hash[0] | (hash[1] << 8) | (hash[2] << 16)
//   let l = (header & 63) / 63
//   let p = ((header >> 6) & 63) / 31.5 - 1
//   let q = ((header >> 12) & 63) / 31.5 - 1
//   let has_alpha = header >> 23
//   let a = has_alpha ? (hash[5] & 15) / 15 : 1
//   let b = l - 2 / 3 * p
//   let r = (3 * l - b + q) / 2
//   let g = r - q
//   return {
//     r: max(0, min(1, r)),
//     g: max(0, min(1, g)),
//     b: max(0, min(1, b)),
//     a
//   }
// }


// /**
//  * Encodes an RGBA image to a PNG data URL. RGB should not be premultiplied by
//  * A. This is optimized for speed and simplicity and does not optimize for size
//  * at all. This doesn't do any compression (all values are stored uncompressed).
//  *
//  * @param w The width of the input image. Must be ≤100px.
//  * @param h The height of the input image. Must be ≤100px.
//  * @param rgba The pixels in the input image, row-by-row. Must have w*h*4 elements.
//  * @returns A data URL containing a PNG for the input image.
//  */
// export function rgbaToDataURL(w, h, rgba) {
//   let row = w * 4 + 1
//   let idat = 6 + h * (5 + row)
//   let bytes = [
//     137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0,
//     w >> 8, w & 255, 0, 0, h >> 8, h & 255, 8, 6, 0, 0, 0, 0, 0, 0, 0,
//     idat >>> 24, (idat >> 16) & 255, (idat >> 8) & 255, idat & 255,
//     73, 68, 65, 84, 120, 1
//   ]
//   let table = [
//     0, 498536548, 997073096, 651767980, 1994146192, 1802195444, 1303535960,
//     1342533948, -306674912, -267414716, -690576408, -882789492, -1687895376,
//     -2032938284, -1609899400, -1111625188
//   ]
//   let a = 1, b = 0
//   for (let y = 0, i = 0, end = row - 1; y < h; y++, end += row - 1) {
//     bytes.push(y + 1 < h ? 0 : 1, row & 255, row >> 8, ~row & 255, (row >> 8) ^ 255, 0)
//     for (b = (b + a) % 65521; i < end; i++) {
//       let u = rgba[i] & 255
//       bytes.push(u)
//       a = (a + u) % 65521
//       b = (b + a) % 65521
//     }
//   }
//   bytes.push(
//     b >> 8, b & 255, a >> 8, a & 255, 0, 0, 0, 0,
//     0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130
//   )
//   for (let [start, end] of [[12, 29], [37, 41 + idat]]) {
//     let c = ~0
//     for (let i = start; i < end; i++) {
//       c ^= bytes[i]
//       c = (c >>> 4) ^ table[c & 15]
//       c = (c >>> 4) ^ table[c & 15]
//     }
//     c = ~c
//     bytes[end++] = c >>> 24
//     bytes[end++] = (c >> 16) & 255
//     bytes[end++] = (c >> 8) & 255
//     bytes[end++] = c & 255
//   }
//   return 'data:image/png;base64,' + btoa(String.fromCharCode(...bytes))
// }

// /**
//  * Decodes a ThumbHash to a PNG data URL. This is a convenience function that
//  * just calls "thumbHashToRGBA" followed by "rgbaToDataURL".
//  *
//  * @param hash The bytes of the ThumbHash.
//  * @returns A data URL containing a PNG for the rendered ThumbHash.
//  */
// export function thumbHashToDataURL(hash) {
//   let image = thumbHashToRGBA(hash)
//   return rgbaToDataURL(image.w, image.h, image.rgba)
// }
