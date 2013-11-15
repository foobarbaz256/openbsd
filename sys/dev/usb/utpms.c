/*	$OpenBSD: utpms.c,v 1.3 2013/11/15 08:17:44 pirofti Exp $	*/

/*
 * Copyright (c) 2005, Johan Wall�n
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the copyright holder may not be used to endorse or
 *    promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * The utpms driver provides support for the trackpad on new (post
 * February 2005) Apple PowerBooks and iBooks that are not standard
 * USB HID mice.
 */

/*
 * The protocol (that is, the interpretation of the data generated by
 * the trackpad) is taken from the Linux appletouch driver version
 * 0.08 by Johannes Berg, Stelian Pop and Frank Arnold.  The method
 * used to detect fingers on the trackpad is also taken from that
 * driver.
 */

/*
 * PROTOCOL:
 *
 * The driver transfers continuously 81 byte events.  The last byte is
 * 1 if the button is pressed, and is 0 otherwise. Of the remaining
 * bytes, 26 + 16 = 42 are sensors detecting pressure in the X or
 * horizontal, and Y or vertical directions, respectively.  On 12 and
 * 15 inch PowerBooks, only the 16 first sensors in the X-direction
 * are used. In the X-direction, the sensors correspond to byte
 * positions
 *
 *   2, 7, 12, 17, 22, 27, 32, 37, 4, 9, 14, 19, 24, 29, 34, 39, 42,
 *   47, 52, 57, 62, 67, 72, 77, 44 and 49;
 *
 * in the Y direction, the sensors correspond to byte positions
 *
 *   1, 6, 11, 16, 21, 26, 31, 36, 3, 8, 13, 18, 23, 28, 33 and 38.
 *
 * The change in the sensor values over time is more interesting than
 * their absolute values: if the pressure increases, we know that the
 * finger has just moved there.
 *
 * We keep track of the previous sample (of sensor values in the X and
 * Y directions) and the accumulated change for each sensor.  When we
 * receive a new sample, we add the difference of the new sensor value
 * and the old value to the accumulated change.  If the accumulator
 * becomes negative, we set it to zero.  The effect is that the
 * accumulator is large for sensors whose pressure has recently
 * increased.  If there is little change in pressure (or if the
 * pressure decreases), the accumulator drifts back to zero.
 *
 * Since there is some fluctuations, we ignore accumulator values
 * below a threshold.  The raw finger position is computed as a
 * weighted average of the other sensors (the weights are the
 * accumulated changes).
 *
 * For smoothing, we keep track of the previous raw finger position,
 * and the virtual position reported to wsmouse.  The new raw position
 * is computed as a weighted average of the old raw position and the
 * computed raw position.  Since this still generates some noise, we
 * compute a new virtual position as a weighted average of the previous
 * virtual position and the new raw position.  The weights are
 * controlled by the raw change and a noise parameter.  The position
 * is reported as a relative position.
 */

/*
 * TODO:
 *
 * Add support for other drivers of the same type.
 *
 * Add support for tapping and two-finger scrolling?  The
 * implementation already detects two fingers, so this should be
 * relatively easy.
 *
 * Implement some of the mouse ioctls?
 *
 * Take care of the XXXs.
 *
 */

#include <sys/param.h>
#include <sys/device.h>
#include <sys/errno.h>

#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/tty.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#include <dev/usb/usbdevs.h>
#include <dev/usb/uhidev.h>

#include <dev/wscons/wsconsio.h>
#include <dev/wscons/wsmousevar.h>

/* The amount of data transferred by the USB device. */
#define UTPMS_DATA_LEN 81

/* The maximum number of sensors. */
#define UTPMS_X_SENSORS 26
#define UTPMS_Y_SENSORS 16
#define UTPMS_SENSORS (UTPMS_X_SENSORS + UTPMS_Y_SENSORS)

/*
 * Parameters for supported devices.  For generality, these parameters
 * can be different for each device.  The meanings of the parameters
 * are as follows.
 *
 * type:      Type of the trackpad device, used for dmesg output, and
 *            to know some of the device parameters.
 *
 * noise:     Amount of noise in the computed position. This controls
 *            how large a change must be to get reported, and how
 *            large enough changes are smoothed.  A good value can
 *            probably only be found experimentally, but something around
 *            16 seems suitable.
 *
 * product:   The product ID of the trackpad.
 *
 *
 * threshold: Accumulated changes less than this are ignored.  A good
 *            value could be determined experimentally, but 5 is a
 *            reasonable guess.
 *
 * vendor:    The vendor ID.  Currently USB_VENDOR_APPLE for all devices.
 *
 * x_factor:  Factor used in computations with X-coordinates.  If the
 *            x-resolution of the display is x, this should be
 *            (x + 1) / (x_sensors - 1).  Other values work fine, but
 *            then the aspect ratio is not necessarily kept.
 *
 * x_sensors: The number of sensors in the X-direction.
 *
 * y_factor:  As x_factors, but for Y-coordinates.
 *
 * y_sensors: The number of sensors in the Y-direction.
 */

struct utpms_dev {
	int type;	   /* Type of the trackpad. */
#define FOUNTAIN	0x00
#define GEYSER1		0x01
#define GEYSER2		0x02
	int noise;	   /* Amount of noise in the computed position. */
	int threshold;	   /* Changes less than this are ignored. */
	int x_factor;	   /* Factor used in computation with X-coordinates. */
	int x_sensors;	   /* The number of X-sensors. */
	int y_factor;	   /* Factor used in computation with Y-coordinates. */
	int y_sensors;	   /* The number of Y-sensors. */
	uint16_t product;  /* Product ID. */
	uint16_t vendor;   /* The vendor ID. */
};

static struct utpms_dev utpms_devices[] = {
#define UTPMS_TOUCHPAD(ttype, prod, x_fact, x_sens, y_fact)		\
       {								\
		.type = (ttype),					\
		.vendor = USB_VENDOR_APPLE,				\
		.product = (prod),					\
		.noise = 16,						\
		.threshold = 5,						\
		.x_factor = (x_fact),					\
		.x_sensors = (x_sens),					\
		.y_factor = (y_fact),					\
		.y_sensors = 16						\
       }
       /* 12 inch PowerBooks */
       UTPMS_TOUCHPAD(FOUNTAIN, 0x030a, 69, 16, 52),
       /* 12 and 14 inch iBook G4 */
       UTPMS_TOUCHPAD(GEYSER1, 0x030b, 69, 16, 52),
       /* 15 inch PowerBooks */
       UTPMS_TOUCHPAD(FOUNTAIN, 0x020e, 85, 16, 57),
       UTPMS_TOUCHPAD(FOUNTAIN, 0x020f, 85, 16, 57),
       UTPMS_TOUCHPAD(GEYSER2, 0x0214, 90, 15, 107),
       UTPMS_TOUCHPAD(GEYSER2, 0x0215, 90, 15, 107),
       UTPMS_TOUCHPAD(GEYSER2, 0x0216, 90, 15, 107),
       /* 17 inch PowerBooks */
       UTPMS_TOUCHPAD(FOUNTAIN, 0x020d, 71, 26, 68),
#undef UTPMS_TOUCHPAD
};

struct utpms_softc {
	struct uhidev sc_hdev;	      /* USB parent (got the struct device). */
	int sc_type;		      /* Type of the trackpad */
	int sc_datalen;
	int sc_acc[UTPMS_SENSORS];     /* Accumulated sensor values. */
	unsigned char sc_prev[UTPMS_SENSORS];   /* Previous sample. */
	unsigned char sc_sample[UTPMS_SENSORS]; /* Current sample. */
	struct device *sc_wsmousedev; /* WSMouse device. */
	int sc_noise;		      /* Amount of noise. */
	int sc_threshold;	      /* Threshold value. */
	int sc_x;		      /* Virtual position in horizontal
				       * direction (wsmouse position). */
	int sc_x_factor;	      /* X-coordinate factor. */
	int sc_x_raw;		      /* X-position of finger on trackpad. */
	int sc_x_sensors;	      /* Number of X-sensors. */
	int sc_y;		      /* Virtual position in vertical direction
				       * (wsmouse position). */
	int sc_y_factor;	      /* Y-coordinate factor. */
	int sc_y_raw;		      /* Y-position of finger on trackpad. */
	int sc_y_sensors;	      /* Number of Y-sensors. */
	uint32_t sc_buttons;	      /* Button state. */
	uint32_t sc_status;	      /* Status flags. */
#define UTPMS_ENABLED 1		      /* Is the device enabled? */
#define UTPMS_VALID 4		      /* Is the previous sample valid? */
};

void	utpms_intr(struct uhidev *, void *, unsigned int);
int	utpms_enable(void *);
void	utpms_disable(void *);
int	utpms_ioctl(void *, unsigned long, caddr_t, int, struct proc *);
void	reorder_sample(struct utpms_softc*, unsigned char *, unsigned char *);
int	compute_delta(struct utpms_softc *, int *, int *, int *, uint32_t *);
int	detect_pos(int *, int, int, int, int *, int *);
int	smooth_pos(int, int, int);

const struct wsmouse_accessops utpms_accessops = {
	utpms_enable,
	utpms_ioctl,
	utpms_disable,
};

int	utpms_match(struct device *, void *, void *);
void	utpms_attach(struct device *, struct device *, void *);
int	utpms_detach(struct device *, int);
int	utpms_activate(struct device *, int);

struct cfdriver utpms_cd = {
	NULL, "utpms", DV_DULL
};

const struct cfattach utpms_ca = {
	sizeof(struct utpms_softc), utpms_match, utpms_attach, utpms_detach,
	utpms_activate,
};

int
utpms_match(struct device *parent, void *match, void *aux)
{
	struct uhidev_attach_arg *uha = (struct uhidev_attach_arg *)aux;
	usb_device_descriptor_t *udd;
	int i;
	uint16_t vendor, product;

	/*
	 * We just check if the vendor and product IDs have the magic numbers
	 * we expect.
	 */
	if ((udd = usbd_get_device_descriptor(uha->parent->sc_udev)) != NULL) {
		vendor = UGETW(udd->idVendor);
		product = UGETW(udd->idProduct);
		for (i = 0; i < nitems(utpms_devices); i++) {
			if (vendor == utpms_devices[i].vendor &&
			    product == utpms_devices[i].product)
				return (UMATCH_IFACECLASS);
		}
	}

	return (UMATCH_NONE);
}

void
utpms_attach(struct device *parent, struct device *self, void *aux)
{
	struct utpms_softc *sc = (struct utpms_softc *)self;
	struct uhidev_attach_arg *uha = (struct uhidev_attach_arg *)aux;
	struct wsmousedev_attach_args a;
	struct utpms_dev *pd;
	usb_device_descriptor_t *udd;
	int i;
	uint16_t vendor, product;

	sc->sc_datalen = UTPMS_DATA_LEN;
	sc->sc_hdev.sc_udev = uha->uaa->device;

	/* Fill in device-specific parameters. */
	if ((udd = usbd_get_device_descriptor(uha->parent->sc_udev)) != NULL) {
		product = UGETW(udd->idProduct);
		vendor = UGETW(udd->idVendor);
		for (i = 0; i < nitems(utpms_devices); i++) {
			pd = &utpms_devices[i];
			if (product == pd->product && vendor == pd->vendor) {
				switch (pd->type) {
				case FOUNTAIN:
					printf(": Fountain");
					break;
				case GEYSER1:
					printf(": Geyser");
					break;
				case GEYSER2:
					sc->sc_type = GEYSER2;
					sc->sc_datalen = 64;
					sc->sc_y_sensors = 9;
					printf(": Geyser 2");
					break;
				}
				printf(" Trackpad\n");
				sc->sc_noise = pd->noise;
				sc->sc_threshold = pd->threshold;
				sc->sc_x_factor = pd->x_factor;
				sc->sc_x_sensors = pd->x_sensors;
				sc->sc_y_factor = pd->y_factor;
				sc->sc_y_sensors = pd->y_sensors;
				break;
			}
		}
	}
	if (sc->sc_x_sensors <= 0 || sc->sc_x_sensors > UTPMS_X_SENSORS ||
	    sc->sc_y_sensors <= 0 || sc->sc_y_sensors > UTPMS_Y_SENSORS) {
		printf(": unexpected sensors configuration (d:d)\n",
		    sc->sc_x_sensors, sc->sc_y_sensors);
		return;
	}

	sc->sc_hdev.sc_intr = utpms_intr;
	sc->sc_hdev.sc_parent = uha->parent;
	sc->sc_hdev.sc_report_id = uha->reportid;

	sc->sc_status = 0;

	a.accessops = &utpms_accessops;
	a.accesscookie = sc;
	sc->sc_wsmousedev = config_found(self, &a, wsmousedevprint);
}

int
utpms_detach(struct device *self, int flags)
{
	struct utpms_softc *sc = (struct utpms_softc *)self;
	int ret = 0;

	/* The wsmouse driver does all the work. */
	if (sc->sc_wsmousedev != NULL)
		ret = config_detach(sc->sc_wsmousedev, flags);

	return (ret);
}

int
utpms_activate(struct device *self, int act)
{
	struct utpms_softc *sc = (struct utpms_softc *)self;
	int ret;

	if (act == DVACT_DEACTIVATE) {
		ret = 0;
		if (sc->sc_wsmousedev != NULL)
			ret = config_deactivate(sc->sc_wsmousedev);
		return (ret);
	}
	return (EOPNOTSUPP);
}

int
utpms_enable(void *v)
{
	struct utpms_softc *sc = v;

	/* Check that we are not detaching or already enabled. */
	if (sc->sc_status & usbd_is_dying(sc->sc_hdev.sc_udev))
		return (EIO);
	if (sc->sc_status & UTPMS_ENABLED)
		return (EBUSY);

	sc->sc_status |= UTPMS_ENABLED;
	sc->sc_status &= ~UTPMS_VALID;
	sc->sc_buttons = 0;
	bzero(sc->sc_sample, sizeof(sc->sc_sample));

	return (uhidev_open(&sc->sc_hdev));
}

void
utpms_disable(void *v)
{
	struct utpms_softc *sc = v;

	if (!(sc->sc_status & UTPMS_ENABLED))
		return;

	sc->sc_status &= ~UTPMS_ENABLED;
	uhidev_close(&sc->sc_hdev);
}

int
utpms_ioctl(void *v, unsigned long cmd, caddr_t data, int flag, struct proc *p)
{
	switch (cmd) {
	case WSMOUSEIO_GTYPE:
		*(u_int *)data = WSMOUSE_TYPE_USB;
		return (0);
	}

	return (-1);
}

void
utpms_intr(struct uhidev *addr, void *ibuf, unsigned int len)
{
	struct utpms_softc *sc = (struct utpms_softc *)addr;
	unsigned char *data;
	int dx, dy, dz, i, s;
	uint32_t buttons;

	/* Ignore incomplete data packets. */
	if (len != sc->sc_datalen)
		return;
	data = ibuf;

	/* The last byte is 1 if the button is pressed and 0 otherwise. */
	buttons = !!data[sc->sc_datalen - 1];

	/* Everything below assumes that the sample is reordered. */
	reorder_sample(sc, sc->sc_sample, data);

	/* Is this the first sample? */
	if (!(sc->sc_status & UTPMS_VALID)) {
		sc->sc_status |= UTPMS_VALID;
		sc->sc_x = sc->sc_y = -1;
		sc->sc_x_raw = sc->sc_y_raw = -1;
		memcpy(sc->sc_prev, sc->sc_sample, sizeof(sc->sc_prev));
		bzero(sc->sc_acc, sizeof(sc->sc_acc));
		return;
	}
	/* Accumulate the sensor change while keeping it nonnegative. */
	for (i = 0; i < UTPMS_SENSORS; i++) {
		sc->sc_acc[i] +=
			(signed char)(sc->sc_sample[i] - sc->sc_prev[i]);

		if (sc->sc_acc[i] < 0)
			sc->sc_acc[i] = 0;
	}
	memcpy(sc->sc_prev, sc->sc_sample, sizeof(sc->sc_prev));

	/* Compute change. */
	dx = dy = dz = 0;
	if (!compute_delta(sc, &dx, &dy, &dz, &buttons))
		return;

	/* Report to wsmouse. */
	if ((dx != 0 || dy != 0 || dz != 0 || buttons != sc->sc_buttons) &&
	    sc->sc_wsmousedev != NULL) {
		s = spltty();
		wsmouse_input(sc->sc_wsmousedev, buttons, dx, -dy, dz, 0,
		    WSMOUSE_INPUT_DELTA);
		splx(s);
	}
	sc->sc_buttons = buttons;
}

/*
 * Reorder the sensor values so that all the X-sensors are before the
 * Y-sensors in the natural order. Note that this might have to be
 * rewritten if UTPMS_X_SENSORS or UTPMS_Y_SENSORS change.
 */
void
reorder_sample(struct utpms_softc *sc, unsigned char *to, unsigned char *from)
{
	int i;

	if (sc->sc_type == GEYSER2) {
		int j;

		bzero(to, UTPMS_SENSORS);
		for (i = 0, j = 19; i < 20; i += 2, j += 3) {
			to[i] = from[j];
			to[i + 1] = from[j + 1];
		}
		for (i = 0, j = 1; i < 9; i += 2, j += 3) {
			to[UTPMS_X_SENSORS + i] = from[j];
			to[UTPMS_X_SENSORS + i + 1] = from[j + 1];
		}
	} else {
		for (i = 0; i < 8; i++) {
			/* X-sensors. */
			to[i] = from[5 * i + 2];
			to[i + 8] = from[5 * i + 4];
			to[i + 16] = from[5 * i + 42];
#if 0
			/*
			 * XXX This seems to introduce random ventical jumps,
			 * so we ignore these sensors until we figure out
			 * their meaning.
			 */
			if (i < 2)
				to[i + 24] = from[5 * i + 44];
#endif /* 0 */
			/* Y-sensors. */
			to[i + 26] = from[5 * i + 1];
			to[i + 34] = from[5 * i + 3];
		}
	}
}

/*
 * Compute the change in x, y and z direction, update the button state
 * (to simulate more than one button, scrolling etc.), and update the
 * history. Note that dx, dy, dz and buttons are modified only if
 * corresponding pressure is detected and should thus be initialised
 * before the call.  Return 0 on error.
 *
 * XXX Could we report something useful in dz?
 */
int
compute_delta(struct utpms_softc *sc, int *dx, int *dy, int *dz,
	      uint32_t * buttons)
{
	int x_det, y_det, x_raw, y_raw, x_fingers, y_fingers, fingers, x, y;

	x_det = detect_pos(sc->sc_acc, sc->sc_x_sensors, sc->sc_threshold,
			   sc->sc_x_factor, &x_raw, &x_fingers);
	y_det = detect_pos(sc->sc_acc + UTPMS_X_SENSORS, sc->sc_y_sensors,
			   sc->sc_threshold, sc->sc_y_factor,
			   &y_raw, &y_fingers);
	fingers = max(x_fingers, y_fingers);

	/* Check the number of fingers and if we have detected a position. */
	if (x_det == 0 && y_det == 0) {
		/* No position detected, resetting. */
		bzero(sc->sc_acc, sizeof(sc->sc_acc));
		sc->sc_x_raw = sc->sc_y_raw = sc->sc_x = sc->sc_y = -1;
	} else if (x_det > 0 && y_det > 0) {
		switch (fingers) {
		case 1:
			/* Smooth position. */
			if (sc->sc_x_raw >= 0) {
				sc->sc_x_raw = (3 * sc->sc_x_raw + x_raw) / 4;
				sc->sc_y_raw = (3 * sc->sc_y_raw + y_raw) / 4;
				/*
				 * Compute virtual position and change if we
				 * already have a decent position.
				 */
				if (sc->sc_x >= 0) {
					x = smooth_pos(sc->sc_x, sc->sc_x_raw,
						       sc->sc_noise);
					y = smooth_pos(sc->sc_y, sc->sc_y_raw,
						       sc->sc_noise);
					*dx = x - sc->sc_x;
					*dy = y - sc->sc_y;
					sc->sc_x = x;
					sc->sc_y = y;
				} else {
					/* Initialise virtual position. */
					sc->sc_x = sc->sc_x_raw;
					sc->sc_y = sc->sc_y_raw;
				}
			} else {
				/* Initialise raw position. */
				sc->sc_x_raw = x_raw;
				sc->sc_y_raw = y_raw;
			}
			break;
		case 2:
			if (*buttons == 1)
				*buttons = 4;
			break;
		case 3:
			if (*buttons == 1)
				*buttons = 2;
			break;
		}
	}
	return (1);
}

/*
 * Compute the new smoothed position from the previous smoothed position
 * and the raw position.
 */
int
smooth_pos(int pos_old, int pos_raw, int noise)
{
	int ad, delta;

	delta = pos_raw - pos_old;
	ad = abs(delta);

	/* Too small changes are ignored. */
	if (ad < noise / 2)
		delta = 0;
	/* A bit larger changes are smoothed. */
	else if (ad < noise)
		delta /= 4;
	else if (ad < 2 * noise)
		delta /= 2;

	return (pos_old + delta);
}

/*
 * Detect the position of the finger.  Returns the total pressure.
 * The position is returned in pos_ret and the number of fingers
 * is returned in fingers_ret.  The position returned in pos_ret
 * is in [0, (n_sensors - 1) * factor - 1].
 */
int
detect_pos(int *sensors, int n_sensors, int threshold, int fact,
	   int *pos_ret, int *fingers_ret)
{
	int i, w, s;

	/*
	 * Compute the number of fingers, total pressure, and weighted
	 * position of the fingers.
	 */
	*fingers_ret = 0;
	w = s = 0;
	for (i = 0; i < n_sensors; i++) {
		if (sensors[i] >= threshold) {
			if (i == 0 || sensors[i - 1] < threshold)
				*fingers_ret += 1;
			s += sensors[i];
			w += sensors[i] * i;
		}
	}

	if (s > 0)
		*pos_ret = w * fact / s;

	return (s);
}
